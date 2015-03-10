/*
 * eap-tls.c - EAP-TLS implementation for PPP
 *
 * Copyright (c) Beniamino Galvani 2005 All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The name(s) of the authors of this software must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission.
 *
 * THE AUTHORS OF THIS SOFTWARE DISCLAIM ALL WARRANTIES WITH REGARD TO
 * THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/x509v3.h>
#include <curl/curl.h>

#include "pppd.h"
#include "eap.h"
#include "eap-tls.h"
#include "pathnames.h"

extern char *crl_dir;
extern bool auto_update_crl;
extern int crl_update_time;

static int search_wordlist(struct wordlist *wl, char *word);
static void add_wordlist(struct wordlist *wl, char *word);
static void destroy_wordlist(struct wordlist *wl);

/*
 * Tests if certificates, key and crl for server use can be loaded.
 */
int eaptls_test_certs_server(char *servcertfile, char *cacertfile,
			 char *pkfile, char *clicertfile)
{
	SSL_CTX *ctx;
	X509_STORE *certstore;
	X509_LOOKUP *lookup;
	X509 *tmp;

	/*
	 * Without these can't continue 
	 */
	if (!cacertfile[0] || !servcertfile[0] || !pkfile[0])
		return 0;

	SSL_library_init();
	SSL_load_error_strings();

	ctx = SSL_CTX_new(TLSv1_method());

	if (!ctx) {
		ERR_print_errors_fp(stderr);
		error("Can't initialize CTX");
		return 0;
	}

	if (!SSL_CTX_load_verify_locations(ctx, cacertfile, NULL))
		goto fail;

	SSL_CTX_set_client_CA_list(ctx, SSL_load_client_CA_file(cacertfile));

	if (!SSL_CTX_use_certificate_file(ctx, servcertfile, SSL_FILETYPE_PEM))
		goto fail;

	if (!SSL_CTX_use_PrivateKey_file(ctx, pkfile, SSL_FILETYPE_PEM))
		goto fail;

	if (SSL_CTX_check_private_key(ctx) != 1)
		goto fail;

	if (crl_dir) {
		if (!(certstore = SSL_CTX_get_cert_store(ctx)))
			goto fail;

		if (!(lookup =
		     X509_STORE_add_lookup(certstore, X509_LOOKUP_hash_dir())))
			goto fail;

		X509_LOOKUP_add_dir(lookup, crl_dir, X509_FILETYPE_PEM);
		X509_STORE_set_flags(certstore, X509_V_FLAG_CRL_CHECK);
	}

	/*
	 * If a client certificate file was specified, it must be valid, else
	 * fail 
	 */
	if (clicertfile[0]) {
		if (!(tmp = get_X509_from_file(clicertfile))) {
			warn("Error loading client certificate from file %s",
			     clicertfile);
			goto fail;
		}
		X509_free(tmp);
	}

	SSL_CTX_free(ctx);

	return 1;

      fail:
	ERR_print_errors_fp(stderr);
	SSL_CTX_free(ctx);
	return 0;
}

/*
 * Tests if certificates, key and crl for client use can be loaded.
 */
int eaptls_test_certs_client(char *clicertfile, char *cacertfile, char *pkfile,
			 char *servcertfile)
{

	SSL_CTX *ctx;
	X509_STORE *certstore;
	X509_LOOKUP *lookup;
	X509 *tmp;

	/*
	 * Without these we can't continue 
	 */
	if (!cacertfile[0] || !clicertfile[0] || !pkfile[0])
		return 0;

	SSL_library_init();
	SSL_load_error_strings();

	ctx = SSL_CTX_new(TLSv1_method());

	if (!ctx) {
		ERR_print_errors_fp(stderr);
		error("Can't initialize CTX");
		return 0;
	}

	if (!SSL_CTX_load_verify_locations(ctx, cacertfile, NULL))
		goto fail;

	if (!SSL_CTX_use_certificate_file(ctx, clicertfile, SSL_FILETYPE_PEM))
		goto fail;

	if (!SSL_CTX_use_PrivateKey_file(ctx, pkfile, SSL_FILETYPE_PEM))
		goto fail;

	if (SSL_CTX_check_private_key(ctx) != 1)
		goto fail;

	if (crl_dir) {

		if (!(certstore = SSL_CTX_get_cert_store(ctx)))
			goto fail;

		if (!(lookup =
		     X509_STORE_add_lookup(certstore, X509_LOOKUP_hash_dir())))
			goto fail;

		X509_LOOKUP_add_dir(lookup, crl_dir, X509_FILETYPE_PEM);
		X509_STORE_set_flags(certstore, X509_V_FLAG_CRL_CHECK);
	}

	/*
	 * If a server certificate file was specified, it must be valid, else
	 * fail 
	 */
	if (servcertfile[0]) {
		if (!(tmp = get_X509_from_file(servcertfile))) {
			warn("Error loading client certificate from file %s",
			     clicertfile);
			goto fail;
		}
		X509_free(tmp);
	}

	SSL_CTX_free(ctx);

	return 1;

      fail:
	ERR_print_errors_fp(stderr);
	SSL_CTX_free(ctx);
	return 0;
}

/*
 * Init the ssl handshake (server mode)
 */
int eaptls_init_ssl_server(eap_state * esp)
{
	struct eaptls_session *ets;
	X509_STORE *certstore;
	X509_LOOKUP *lookup;
	char servcertfile[MAXWORDLEN];
	char clicertfile[MAXWORDLEN];
	char cacertfile[MAXWORDLEN];
	char pkfile[MAXWORDLEN];
	struct wordlist updatedfiles;

	updatedfiles.next = NULL;
	updatedfiles.word = NULL;

	/*
	 * Allocate new eaptls session 
	 */
	esp->es_server.ea_session = malloc(sizeof(struct eaptls_session));
	if (!esp->es_server.ea_session)
		fatal("Allocation error");
	ets = esp->es_server.ea_session;

	if (!esp->es_server.ea_peer) {
		error("Error: client name not set (BUG)");
		return 0;
	}

	strncpy(ets->peer, esp->es_server.ea_peer, MAXWORDLEN);

	if (!get_eaptls_secret(esp->es_unit, esp->es_server.ea_peer,
			       esp->es_server.ea_name, clicertfile,
			       servcertfile, cacertfile, pkfile, 1)) {
		dbglog("get_eaptls_secret error");
		return 0;
	}

	ets->mtu = netif_get_mtu(esp->es_unit) - PPP_HDRLEN - 10;

	SSL_library_init();
	SSL_load_error_strings();

	ets->ctx = SSL_CTX_new(TLSv1_method());

	if (!ets->ctx) {
		ERR_print_errors_fp(stderr);
		error("Can't initialize CTX");
		return 0;
	}

	if (!SSL_CTX_load_verify_locations(ets->ctx, cacertfile, NULL))
		goto fail;

	SSL_CTX_set_client_CA_list(ets->ctx,
				   SSL_load_client_CA_file(cacertfile));

	if (!SSL_CTX_use_certificate_file
	    (ets->ctx, servcertfile, SSL_FILETYPE_PEM))
		goto fail;

	if (!SSL_CTX_use_PrivateKey_file(ets->ctx, pkfile, SSL_FILETYPE_PEM))
		goto fail;

	if (SSL_CTX_check_private_key(ets->ctx) != 1)
		goto fail;

	SSL_CTX_set_options(ets->ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3);
	SSL_CTX_set_verify_depth(ets->ctx, 5);
	SSL_CTX_set_verify(ets->ctx,
			   SSL_VERIFY_PEER |
			   SSL_VERIFY_FAIL_IF_NO_PEER_CERT,
			   &ssl_verify_callback);

	if (crl_dir) {
		if (!(certstore = SSL_CTX_get_cert_store(ets->ctx)))
			goto fail;

		if (!(lookup = X509_STORE_add_lookup(certstore, 
				X509_LOOKUP_hash_dir())))
			goto fail;

		if (auto_update_crl)
			eaptls_update_crls(crl_dir, cacertfile, &updatedfiles);

		destroy_wordlist(&updatedfiles);

		X509_LOOKUP_add_dir(lookup, crl_dir, X509_FILETYPE_PEM);
		X509_STORE_set_flags(certstore, X509_V_FLAG_CRL_CHECK);
	}

	if (!(ets->ssl = SSL_new(ets->ctx)))
		goto fail;

	/*
	 * Initialize the BIOs we use to read/write to ssl engine 
	 */
	ets->into_ssl = BIO_new(BIO_s_mem());
	ets->from_ssl = BIO_new(BIO_s_mem());
	SSL_set_bio(ets->ssl, ets->into_ssl, ets->from_ssl);

	SSL_set_msg_callback(ets->ssl, ssl_msg_callback);
	SSL_set_msg_callback_arg(ets->ssl, ets);

	/*
	 * Attach the session struct to the connection, so we can later
	 * retrieve it when doing certificate verification
	 */
	SSL_set_ex_data(ets->ssl, 0, ets);

	SSL_set_accept_state(ets->ssl);

	ets->data = NULL;
	ets->datalen = 0;
	ets->alert_sent = 0;
	ets->alert_recv = 0;

	/*
	 * If we specified the client certificate file, store it in ets->peercertfile,
	 * so we can check it later in ssl_verify_callback()
	 */
	if (clicertfile[0])
		strncpy(&ets->peercertfile[0], clicertfile, MAXWORDLEN);
	else
		ets->peercertfile[0] = 0;

	return 1;

      fail:
	ERR_print_errors_fp(stderr);
	SSL_CTX_free(ets->ctx);
	return 0;
}

/*
 * Init the ssl handshake (client mode)
 */
int eaptls_init_ssl_client(eap_state * esp)
{
	struct eaptls_session *ets;
	X509_STORE *certstore;
	X509_LOOKUP *lookup;
	char servcertfile[MAXWORDLEN];
	char clicertfile[MAXWORDLEN];
	char cacertfile[MAXWORDLEN];
	char pkfile[MAXWORDLEN];
	struct wordlist updatedfiles;

	updatedfiles.next = NULL;
	updatedfiles.word = NULL;

	/*
	 * Allocate new eaptls session 
	 */
	esp->es_client.ea_session = malloc(sizeof(struct eaptls_session));
	if (!esp->es_client.ea_session)
		fatal("Allocation error");
	ets = esp->es_client.ea_session;

	/*
	 * If available, copy server name in ets; it will be used in cert
	 * verify 
	 */
	if (esp->es_client.ea_peer)
		strncpy(ets->peer, esp->es_client.ea_peer, MAXWORDLEN);
	else
		ets->peer[0] = 0;

	ets->mtu = netif_get_mtu(esp->es_unit) - PPP_HDRLEN - 10;

	if (!get_eaptls_secret(esp->es_unit, esp->es_client.ea_name,
			       esp->es_client.ea_peer, clicertfile,
			       servcertfile, cacertfile, pkfile, 0)) {
		dbglog("get_eaptls_secret error");
		return 0;
	}

	SSL_library_init();
	SSL_load_error_strings();

	ets->ctx = SSL_CTX_new(TLSv1_method());

	if (!ets->ctx) {
		ERR_print_errors_fp(stderr);
		error("Can't initialize CTX");
		return 0;
	}

	if (!SSL_CTX_load_verify_locations(ets->ctx, cacertfile, NULL))
		goto fail;

	if (!SSL_CTX_use_certificate_file
	    (ets->ctx, clicertfile, SSL_FILETYPE_PEM))
		goto fail;

	if (!SSL_CTX_use_PrivateKey_file(ets->ctx, pkfile, SSL_FILETYPE_PEM))
		goto fail;

	if (SSL_CTX_check_private_key(ets->ctx) != 1)
		goto fail;

	SSL_CTX_set_options(ets->ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3);
	SSL_CTX_set_verify_depth(ets->ctx, 5);
	SSL_CTX_set_verify(ets->ctx, SSL_VERIFY_PEER, &ssl_verify_callback);

	if (crl_dir) {
		if (!(certstore = SSL_CTX_get_cert_store(ets->ctx)))
			goto fail;

		if (!(lookup = X509_STORE_add_lookup(certstore, 
				X509_LOOKUP_hash_dir())))
			goto fail;

		if (auto_update_crl)
			eaptls_update_crls(crl_dir, cacertfile, &updatedfiles);

		destroy_wordlist(&updatedfiles);

		X509_LOOKUP_add_dir(lookup, crl_dir, X509_FILETYPE_PEM);
		X509_STORE_set_flags(certstore, X509_V_FLAG_CRL_CHECK);
	}

	ets->ssl = SSL_new(ets->ctx);

	if (!ets->ssl)
		goto fail;

	/*
	 * Initialize the BIOs we use to read/write to ssl engine 
	 */
	ets->into_ssl = BIO_new(BIO_s_mem());
	ets->from_ssl = BIO_new(BIO_s_mem());
	SSL_set_bio(ets->ssl, ets->into_ssl, ets->from_ssl);

	SSL_set_msg_callback(ets->ssl, ssl_msg_callback);
	SSL_set_msg_callback_arg(ets->ssl, ets);

	/*
	 * Attach the session struct to the connection, so we can later
	 * retrieve it when doing certificate verification
	 */
	SSL_set_ex_data(ets->ssl, 0, ets);

	SSL_set_connect_state(ets->ssl);

	ets->data = NULL;
	ets->datalen = 0;
	ets->alert_sent = 0;
	ets->alert_recv = 0;

	/*
	 * If we specified the server certificate file, store it in
	 * ets->peercertfile, so we can check it later in
	 * ssl_verify_callback() 
	 */
	if (servcertfile[0])
		strncpy(ets->peercertfile, servcertfile, MAXWORDLEN);
	else
		ets->peercertfile[0] = 0;

	return 1;
      fail:
	ERR_print_errors_fp(stderr);
	SSL_CTX_free(ets->ctx);
	return 0;

}

/*
 * Updates the CA CRL
 */
void eaptls_update_crls(char *crldir, char *cacertfile, struct wordlist *uf)
{
	FILE *file;
	X509 *cert;
	char *filename;

	if (!(file = fopen(cacertfile, "r")))
		return;

	filename = malloc(strlen(crldir) + 15);

	/*
	 * for each CA certificate, control if the CRL must be updated 
	 */
	while ((cert = PEM_read_X509(file, NULL, NULL, NULL))) {

		if (eaptls_must_update(cert, crldir, filename, uf) == 1) {

			eaptls_get_crl(cert, filename);
			add_wordlist(uf, filename);
		}

	}

	free(filename);
	fclose(file);

}

/*
 * Control if the CA CRL must be updated
 * Return in filename the file to be updated 
 */
int eaptls_must_update(X509 * cert, char *crldir, char *filename,
		   struct wordlist *uf)
{
	X509_NAME *xn;
	unsigned long hash;
	int found = 0, i, hours;
	FILE *file;
	X509_CRL *crl;
	time_t curr_time, mod_time;
	struct stat file_stat;
	char tmpname[15];
	char subject[256];
	char cn_str[256];

	/*
	 * Get CA subject name and calculate its hash 
	 */
	xn = X509_get_subject_name(cert);
	hash = X509_NAME_hash(xn);

	X509_NAME_oneline(xn, subject, 256);
	X509_NAME_get_text_by_NID(xn, NID_commonName, cn_str, 256);
	dbglog("  -> processing CA: %s", cn_str);

	/*
	 * Search a CRL for this CA in crldir 
	 */
	for (i = 0; i < 10; i++) {
		sprintf(tmpname, "%08lx.r%1d", hash, i);
		strcpy(filename, crldir);
		strcat(filename, "/");
		strcat(filename, tmpname);

		if (!(file = fopen(filename, "r")))
			break;	/* CRL not found */

		if (!(crl = PEM_read_X509_CRL(file, NULL, NULL, NULL))) {
			fclose(file);
			continue;
		}
		fclose(file);

		/*
		 * test if it's the right CRL 
		 */
		if (X509_NAME_cmp(xn, X509_CRL_get_issuer(crl)) == 0) {
			found = 1;
			X509_CRL_free(crl);
			break;
		}
		X509_CRL_free(crl);
	}

	/*
	 * If file already in list, we've already updated it 
	 */
	if (search_wordlist(uf, filename) == 1)
		return 0;

	/*
	 * If CRL file for this CA not found, need update 
	 */
	if (!found)
		return 1;

	curr_time = time(NULL);
	stat(filename, &file_stat);
	mod_time = file_stat.st_mtime;
	hours = (curr_time - mod_time) / 3600;

	/*
	 * If file too old, need update 
	 */
	if (hours >= crl_update_time) {
		dbglog("        file %s is %d hours old, need update",
		       filename, hours);
		return 1;
	}

	return 0;
}

/*
 * Extract CDPs from certificate 
 */
void eaptls_get_crl(X509 * cert, char *filename)
{
	STACK_OF(DIST_POINT) * distpoints = NULL;
	DIST_POINT *dp;
	DIST_POINT_NAME *dpn;
	STACK_OF(GENERAL_NAME) * names;
	GENERAL_NAME *name;
	ASN1_IA5STRING *uri_asn;
	char *uri;
	int num_dp, i;
	int num_name, j;

	distpoints = X509_get_ext_d2i(cert, 
			NID_crl_distribution_points, NULL, NULL);
	num_dp = sk_DIST_POINT_num(distpoints);

	dbglog("        CA certificate contains %d CDP", num_dp);

	for (i = 0; i < num_dp; i++) {

		dp = sk_DIST_POINT_value(distpoints, i);
		dpn = dp->distpoint;
		names = dpn->name.fullname;
		num_name = sk_GENERAL_NAME_num(names);

		for (j = 0; j < num_name; j++) {

			name = sk_GENERAL_NAME_value(names, j);

			if (name->type == GEN_URI) {
				uri_asn = name->d.uniformResourceIdentifier;
				uri = ASN1_STRING_data(uri_asn);

				if (eaptls_download_crl(uri, filename)) {
					dbglog
					    ("          * %s successfully loaded",
					     uri);
					return;
				}

			}

		}
	}
}

/*
 * Download a CRL from uri into file 
 */
int eaptls_download_crl(char *uri, char *filename)
{
	FILE *tmpfile;
	FILE *destfile;
	int tmpfd;
	char tmpname[30];
	CURL *curl;
	int res;
	X509_CRL *crl;

	strcpy(tmpname, "/tmp/ppp-tmp-XXXXXX");

	if ((tmpfd = mkstemp(tmpname)) == -1) {
		error("error creating temp file /tmp/ppp-tmp-*");
		return 0;
	}

	tmpfile = fdopen(tmpfd, "w");
	if (!tmpfile) {
		error("error fdopen");
		close(tmpfd);
		return 0;
	}

	/*
	 * download in temp file using libcurl 
	 */
	curl_global_init(CURL_GLOBAL_NOTHING);
	curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, uri);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, tmpfile);

	res = curl_easy_perform(curl);

	curl_easy_cleanup(curl);
	curl_global_cleanup();

	if (res != CURLE_OK) {
		dbglog("          * %s download failed", uri);
		fclose(tmpfile);
		unlink(tmpname);
		return 0;
	}

	fclose(tmpfile);
	tmpfile = fopen(tmpname, "r");
	if(!tmpfile)
		return 0;

	/*
	 * load CRL from temp file 
	 */
	crl = d2i_X509_CRL_fp(tmpfile, NULL);

	fclose(tmpfile);
	unlink(tmpname);

	if (!crl) {
		warn("cannot load DER CRL");
		return 0;
	}

	destfile = fopen(filename, "w");
	if (!destfile) {
		X509_CRL_free(crl);
		return 0;
	}

	/*
	 * write CRL in PEM format to destination file 
	 */
	PEM_write_X509_CRL(destfile, crl);

	X509_CRL_free(crl);
	fclose(destfile);

	return 1;
}

void eaptls_free_session(struct eaptls_session *ets)
{
	if (ets->ssl)
		SSL_free(ets->ssl);

	if (ets->ctx)
		SSL_CTX_free(ets->ctx);

	free(ets);
}

/*
 * Handle a received packet, reassembling fragmented messages and
 * passing them to the ssl engine
 */
int eaptls_receive(struct eaptls_session *ets, u_char * inp, int len)
{
	u_char flags;
	u_int tlslen;
	u_char dummy[65536];

	GETCHAR(flags, inp);
	len--;

	if (flags & EAP_TLS_FLAGS_LI) {

		/*
		 * This is the first packet of a message 
		 */

		if (ets->data) {
			warn("EAP TLS: unexpected first fragment");
			return 1;
		}

		GETLONG(tlslen, inp);
		len -= 4;

		if (tlslen > EAP_TLS_MAX_LEN) {
			error("Error: tls message length > %d, truncated",
			      EAP_TLS_MAX_LEN);
			tlslen = EAP_TLS_MAX_LEN;
		}

		/*
		 * Allocate memory for the whole message 
		 */
		ets->data = malloc(tlslen);
		if (!ets->data)
			fatal("EAP TLS: allocation error\n");

		ets->datalen = 0;
		ets->tlslen = tlslen;

	} else if (!ets->data) {

		/*
		 * A non fragmented message without LI flag 
		 */

		ets->data = malloc(len);
		if (!ets->data)
			fatal("EAP TLS: allocation error\n");

		ets->datalen = 0;
		ets->tlslen = len;
	}

	if (flags & EAP_TLS_FLAGS_MF)
		ets->frag = 1;
	else
		ets->frag = 0;

	if (len + ets->datalen > ets->tlslen) {
		warn("EAP TLS: received data > TLS message length");
		return 1;
	}

	BCOPY(inp, ets->data + ets->datalen, len);
	ets->datalen += len;

	if (!ets->frag) {

		/*
		 * If we have the whole message, pass it to ssl 
		 */

		if (ets->datalen != ets->tlslen) {
			warn("EAP TLS: received data != TLS message length");
			return 1;
		}

		if (BIO_write(ets->into_ssl, ets->data, ets->datalen) == -1)
			ERR_print_errors_fp(stderr);

		SSL_read(ets->ssl, dummy, 65536);

		free(ets->data);
		ets->data = NULL;
		ets->datalen = 0;
	}

	return 0;
}

/*
 * Return an eap-tls packet in outp.
 * A TLS message read from the ssl engine is buffered in ets->data.
 * At each call we control if there is buffered data and send a 
 * packet of mtu bytes.
 */
int eaptls_send(struct eaptls_session *ets, u_char ** outp)
{
	bool first = 0;
	int size;
	u_char fromtls[65536];
	int res;
	u_char *start;

	start = *outp;

	if (!ets->data) {

		if(!ets->alert_sent)
			SSL_read(ets->ssl, fromtls, 65536);

		/*
		 * Read from ssl 
		 */
		if ((res = BIO_read(ets->from_ssl, fromtls, 65536)) == -1)
			fatal("No data from BIO_read");

		ets->datalen = res;

		ets->data = malloc(ets->datalen);
		BCOPY(fromtls, ets->data, ets->datalen);

		ets->offset = 0;
		first = 1;

	}

	size = ets->datalen - ets->offset;

	if (size > ets->mtu) {
		size = ets->mtu;
		ets->frag = 1;
	} else
		ets->frag = 0;

	PUTCHAR(EAPT_TLS, *outp);

	/*
	 * Set right flags and length if necessary 
	 */
	if (ets->frag && first) {
		PUTCHAR(EAP_TLS_FLAGS_LI | EAP_TLS_FLAGS_MF, *outp);
		PUTLONG(ets->datalen, *outp);
	} else if (ets->frag) {
		PUTCHAR(EAP_TLS_FLAGS_MF, *outp);
	} else
		PUTCHAR(0, *outp);

	/*
	 * Copy the data in outp 
	 */
	BCOPY(ets->data + ets->offset, *outp, size);
	INCPTR(size, *outp);

	/*
	 * Copy the packet in retransmission buffer 
	 */
	BCOPY(start, &ets->rtx[0], *outp - start);
	ets->rtx_len = *outp - start;

	ets->offset += size;

	if (ets->offset >= ets->datalen) {

		/*
		 * The whole message has been sent 
		 */

		free(ets->data);
		ets->data = NULL;
		ets->datalen = 0;
		ets->offset = 0;
	}

	return 0;
}

/*
 * Get the sent packet from the retransmission buffer
 */
void eaptls_retransmit(struct eaptls_session *ets, u_char ** outp)
{
	BCOPY(ets->rtx, *outp, ets->rtx_len);
	INCPTR(ets->rtx_len, *outp);
}

/*
 * Verify a certificate.
 * Most of the work (signatures and issuer attributes checking)
 * is done by ssl; we check the CN in the peer certificate 
 * against the peer name.
 */
int ssl_verify_callback(int preverify_ok, X509_STORE_CTX * ctx)
{
	char subject[256];
	char cn_str[256];
	X509 *peer_cert;
	int err, depth;
	int ok = preverify_ok;
	SSL *ssl;
	struct eaptls_session *ets;

	peer_cert = X509_STORE_CTX_get_current_cert(ctx);
	err = X509_STORE_CTX_get_error(ctx);
	depth = X509_STORE_CTX_get_error_depth(ctx);

	dbglog("certificate verify depth: %d", depth);

	if (!ok) {
		X509_NAME_oneline(X509_get_subject_name(peer_cert),
				  subject, 256);

		X509_NAME_get_text_by_NID(X509_get_subject_name(peer_cert),
					  NID_commonName, cn_str, 256);

		dbglog("Certificate verification error:\n depth: %d CN: %s"
		       "\n err: %d (%s)\n", depth, cn_str, err,
		       X509_verify_cert_error_string(err));

		return 0;
	}

	ssl = X509_STORE_CTX_get_ex_data(ctx,
				       SSL_get_ex_data_X509_STORE_CTX_idx());

	ets = (struct eaptls_session *)SSL_get_ex_data(ssl, 0);

	if (ets == NULL) {
		error("Error: SSL_get_ex_data returned NULL");
		return 0;
	}

	ERR_print_errors_fp(stderr);

	if (!depth) {		/* This is the peer certificate */

		X509_NAME_oneline(X509_get_subject_name(peer_cert),
				  subject, 256);

		X509_NAME_get_text_by_NID(X509_get_subject_name(peer_cert),
					  NID_commonName, cn_str, 256);

		/*
		 * If acting as client and the name of the server wasn't specified
		 * explicitely, we can't verify the server authenticity 
		 */
		if (!ets->peer[0]) {
			warn("Peer name not specified: no check");
			return 1;
		}

		/*
		 * Check the CN 
		 */
		if (strcmp(cn_str, ets->peer)) {
			error
			    ("Certificate verification error: CN (%s) != peer_name (%s)",
			     cn_str, ets->peer);
			return 0;
		}

		warn("Certificate CN: %s , peer name %s", cn_str, ets->peer);

		/*
		 * If a peer certificate file was specified, here we check it 
		 */
		if (ets->peercertfile[0]) {
			if (ssl_cmp_certs(&ets->peercertfile[0], peer_cert)
			    != 0) {
				error
				    ("Peer certificate doesn't match stored certificate");
				return 0;
			}
		}
	}

	return 1;
}

/*
 * Compare a certificate with the one stored in a file
 */
int ssl_cmp_certs(char *filename, X509 * a)
{
	X509 *b;
	int ret;

	if (!(b = get_X509_from_file(filename)))
		return 1;

	ret = X509_cmp(a, b);
	X509_free(b);

	return ret;

}

X509 *get_X509_from_file(char *filename)
{
	FILE *fp;
	X509 *ret;

	if (!(fp = fopen(filename, "r")))
		return NULL;

	ret = PEM_read_X509(fp, NULL, NULL, NULL);

	fclose(fp);

	return ret;
}

/*
 * Every sent & received message this callback function is invoked,
 * so we know when alert messages have arrived or are sent and
 * we can print debug information about TLS handshake.
 */
void
ssl_msg_callback(int write_p, int version, int content_type,
		 const void *buf, size_t len, SSL * ssl, void *arg)
{
	char string[256];
	struct eaptls_session *ets = (struct eaptls_session *)arg;
	unsigned char code;

	if(write_p)
		strcpy(string, " -> ");
	else
		strcpy(string, " <- ");

	
	switch(content_type) {

	case SSL3_RT_ALERT:	
		strcat(string, "Alert: ");	
		code = ((const unsigned char *)buf)[1];

		if (write_p) {
			ets->alert_sent = 1;
			ets->alert_sent_desc = code;
		} else {
			ets->alert_recv = 1;
			ets->alert_recv_desc = code;
		}

		strcat(string, SSL_alert_desc_string_long(code));
		break;

	case SSL3_RT_CHANGE_CIPHER_SPEC:
		strcat(string, "ChangeCipherSpec");
		break;

	case SSL3_RT_HANDSHAKE:

		strcat(string, "Handshake: ");
		code = ((const unsigned char *)buf)[0];

		switch(code) {
                        case SSL3_MT_HELLO_REQUEST:
                                strcat(string,"Hello Request");
                                break;
                        case SSL3_MT_CLIENT_HELLO:
				strcat(string,"Client Hello");
                                break;
                        case SSL3_MT_SERVER_HELLO:
				strcat(string,"Server Hello");
                                break;
                        case SSL3_MT_CERTIFICATE:
				strcat(string,"Certificate");
                                break;
                        case SSL3_MT_SERVER_KEY_EXCHANGE:
				strcat(string,"Server Key Exchange");
                                break;
                        case SSL3_MT_CERTIFICATE_REQUEST:
				strcat(string,"Certificate Request");
                                break;
                        case SSL3_MT_SERVER_DONE:
				strcat(string,"Server Hello Done");
                                break;
                        case SSL3_MT_CERTIFICATE_VERIFY:
				strcat(string,"Certificate Verify");
                                break;
                        case SSL3_MT_CLIENT_KEY_EXCHANGE:
				strcat(string,"Client Key Exchange");
                                break;
                        case SSL3_MT_FINISHED:
				strcat(string,"Finished");
                                break;
		}
		break;
	}

	/* Alert messages must always be displayed */
	if(content_type == SSL3_RT_ALERT)
		error("%s", string);
	else
		dbglog("%s", string);
}

/*
 * Extract all CA files from conf file 'filename' and update crl
 */
void eaptls_do_update(char *filename, struct wordlist *updatedfiles)
{
	int newline;
	FILE *f;
	char word[MAXWORDLEN];

	if (!(f = fopen(filename, "r"))) {
		fprintf(stderr, "can't open %s", filename);
		die(1);
	}

	if (!getword(f, word, &newline, filename))
		return;		/* file is empty??? */

	newline = 1;

	for (;;) {

		/*
		 * Skip until we find a word at the start of a line.
		 */
		while (!newline && getword(f, word, &newline, filename)) ;
		if (!newline)
			break;	/* got to end of file */

		/*
		 * server 
		 */
		if (!getword(f, word, &newline, filename))
			break;
		if (newline)
			continue;

		/*
		 * client cert 
		 */
		if (!getword(f, word, &newline, filename))
			break;
		if (newline)
			continue;

		/*
		 * server cert 
		 */
		if (!getword(f, word, &newline, filename))
			break;
		if (newline)
			continue;

		/*
		 * ca cert 
		 */
		if (!getword(f, word, &newline, filename))
			break;
		if (newline)
			continue;

		printf("Updating CRLs for CA file %s\n", word);

		eaptls_update_crls(crl_dir, word, updatedfiles);
	}

	fclose(f);
}

void eaptls_only_update_crl(int server, int client)
{
	/*
	 * mantain a list of updated crl files, so if a CA appears more
	 * times, it is updated once 
	 */
	struct wordlist updatedfiles;

	updatedfiles.next = NULL;
	updatedfiles.word = NULL;

	if (server)
		eaptls_do_update(_PATH_EAPTLSSERVFILE, &updatedfiles);

	if (client)
		eaptls_do_update(_PATH_EAPTLSCLIFILE, &updatedfiles);

	destroy_wordlist(&updatedfiles);
}

static int search_wordlist(struct wordlist *wl, char *word)
{
	struct wordlist *tmp = wl->next;

	while (tmp) {
		if (strcmp(tmp->word, word) == 0)
			return 1;
		tmp = tmp->next;
	}
	return 0;
}

static void add_wordlist(struct wordlist *wl, char *word)
{
	struct wordlist *tmp = wl;

	while (tmp->next)
		tmp = tmp->next;

	tmp->next = malloc(sizeof(struct wordlist));
	tmp->next->word = strdup(word);
	tmp->next->next = NULL;

}

static void destroy_wordlist(struct wordlist *wl)
{
	struct wordlist *tmp = wl->next;
	struct wordlist *next;

	while (tmp) {
		next = tmp->next;

		if (tmp->word)
			free(tmp->word);

		free(tmp);
		tmp = next;
	}

}
