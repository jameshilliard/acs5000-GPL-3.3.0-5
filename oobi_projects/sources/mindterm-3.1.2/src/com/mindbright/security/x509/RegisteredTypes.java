/******************************************************************************
 *
 * Copyright (c) 1999-2005 AppGate Network Security AB. All Rights Reserved.
 * 
 * This file contains Original Code and/or Modifications of Original Code as
 * defined in and that are subject to the MindTerm Public Source License,
 * Version 2.0, (the 'License'). You may not use this file except in compliance
 * with the License.
 * 
 * You should have received a copy of the MindTerm Public Source License
 * along with this software; see the file LICENSE.  If not, write to
 * AppGate Network Security AB, Otterhallegatan 2, SE-41118 Goteborg, SWEDEN
 *
 *****************************************************************************/

package com.mindbright.security.x509;

import com.mindbright.asn1.ASN1OIDRegistry;

public class RegisteredTypes extends ASN1OIDRegistry {

    public RegisteredTypes() {
        /*
         * 2.5.29. - certificateExtension
         *
         * From rfc2459:
         *   Conforming CAs MUST support key identifiers (see sec. 4.2.1.1 and
         *   4.2.1.2), basic constraints (see sec. 4.2.1.10), key usage (see
         *   sec.  4.2.1.3), and certificate policies (see sec. 4.2.1.5)
         *   extensions. If the CA issues certificates with an empty sequence
         *   for the subject field, the CA MUST support the subject alternative
         *   name extension
         *
         *   At a minimum, applications conforming to this profile MUST
         *   recognize the extensions which must or may be critical in this
         *   specification.  These extensions are: key usage (see sec. 4.2.1.3),
         *   certificate policies (see sec. 4.2.1.5), the subject alternative
         *   name (see sec.  4.2.1.7), basic constraints (see sec. 4.2.1.10),
         *   name constraints (see sec. 4.2.1.11), policy constraints (see
         *   sec. 4.2.1.12), and extended key usage (see sec. 4.2.1.13).
         */
        put("2.5.29.14", "com.mindbright.security.x509.SubjectKeyIdentifier");
        put("2.5.29.15", "com.mindbright.security.x509.KeyUsage");
        put("2.5.29.16", "com.mindbright.security.x509.PrivateKeyUsagePeriod");
        put("2.5.29.17", "com.mindbright.security.x509.SubjectAltName");
        put("2.5.29.18", "com.mindbright.security.x509.IssuerAltName");
        put("2.5.29.19", "com.mindbright.security.x509.BasicConstraints");
        put("2.5.29.30", "com.mindbright.security.x509.NameConstraints");
        put("2.5.29.32", "com.mindbright.security.x509.CertificatePolicies");
        put("2.5.29.33", "com.mindbright.security.x509.PolicyMappings");
        put("2.5.29.35", "com.mindbright.security.x509.AuthorityKeyIdentifier");
        put("2.5.29.36", "com.mindbright.security.x509.PolicyConstraints");

        /*
         * 2.5.4. - X.500 attribute types
         * (from http://www.alvestrand.no/objectid/2.5.4.html)
         *
         * From rfc2459:
         * SupportedAttributes     ATTRIBUTE       ::=     {
         *       name | commonName | surname | givenName | initials |
         *       generationQualifier | dnQualifier | countryName |
         *       localityName | stateOrProvinceName | organizationName |
         *       organizationalUnitName | title | pkcs9email }
         */

        // commonName
        put("2.5.4.3", "com.mindbright.security.x509.DirectoryString");
        // surname
        put("2.5.4.4", "com.mindbright.security.x509.DirectoryString");
        // serialNumber
        put("2.5.4.5", "ASN1PrintableString");
        // countryName
        put("2.5.4.6", "ASN1PrintableString");
        // localityName
        put("2.5.4.7", "com.mindbright.security.x509.DirectoryString");
        // stateOrProvinceName
        put("2.5.4.8", "com.mindbright.security.x509.DirectoryString");
        // streetAddress
        put("2.5.4.9", "com.mindbright.security.x509.DirectoryString");
        // organizationName
        put("2.5.4.10", "com.mindbright.security.x509.DirectoryString");
        // organizationalUnitName
        put("2.5.4.11", "com.mindbright.security.x509.DirectoryString");
        // title
        put("2.5.4.12", "com.mindbright.security.x509.DirectoryString");
        // description
        put("2.5.4.13", "com.mindbright.security.x509.DirectoryString");
        // businessCategory
        put("2.5.4.15", "com.mindbright.security.x509.DirectoryString");
        // postalAddress
        put("2.5.4.16", "com.mindbright.security.x509.PostalAddress");
        // postalCode
        put("2.5.4.17", "com.mindbright.security.x509.DirectoryString");
        // postOfficeBox
        put("2.5.4.18", "com.mindbright.security.x509.DirectoryString");
        // telephoneNumber
        put("2.5.4.20", "ASN1PrintableString");
        // name
        put("2.5.4.41", "com.mindbright.security.x509.DirectoryString");
        // givenName
        put("2.5.4.42", "com.mindbright.security.x509.DirectoryString");
        // initials
        put("2.5.4.43", "com.mindbright.security.x509.DirectoryString");
        // generationQualifier
        put("2.5.4.44", "com.mindbright.security.x509.DirectoryString");
        // uniqueIdentifier
        put("2.5.4.45", "ASN1BitString");
        // dnQualifier
        put("2.5.4.46", "ASN1PrintableString");
        // pseudonym
        put("2.5.4.65", "com.mindbright.security.x509.DirectoryString");

        // pkcs-9-at-emailAddress
        put("1.2.840.113549.1.9.1", "ASN1IA5String");

        /*
         * PKIX personal data attributes
         * (from rfc3039, Qualified Certificates Profile)
         */
        // dateOfBirth
        put("1.3.6.1.5.5.7.9.1", "ASN1GeneralizedTime");
        // placeOfBirth
        put("1.3.6.1.5.5.7.9.2", "com.mindbright.security.x509.DirectoryString");
        // gender
        put("1.3.6.1.5.5.7.9.3", "ASN1PrintableString");
        // countryOfCitizenship
        put("1.3.6.1.5.5.7.9.4", "ASN1PrintableString");
        // countryOfResidence
        put("1.3.6.1.5.5.7.9.5", "ASN1PrintableString");

        /*
         * According to rfc2459: ...In addition, implementations of this
         * specification MUST be prepared to receive the domainComponent
         * attribute, as defined in [RFC 2247].
         */
        put("0.9.2342.19200300.100.1.25", "ASN1IA5String");
        /*
         * Some seem to use this deprecated rfc822Mailbox attribute, also from
         * the UCL X.500 pilot (not mentioned in any recent standard I know of
         * though).
         */
        put("0.9.2342.19200300.100.1.3", "ASN1IA5String");

        put("1.2.840.10040.4.1","com.mindbright.security.pkcs1.DSAParams");

        putName("2.5.4.3", "commonName", "CN");
        putName("2.5.4.4", "surname", "S");
        putName("2.5.4.5", "serialNumber");
        putName("2.5.4.6", "countryName", "C");
        putName("2.5.4.7", "localityName", "L");
        putName("2.5.4.8", "stateOrProvinceName", "ST");
        putName("2.5.4.9", "streetAddress", "STREET");
        putName("2.5.4.10", "organizationName", "O");
        putName("2.5.4.11", "organizationalUnitName", "OU");
        putName("2.5.4.12", "title", "T");
        putName("2.5.4.13", "description", "D");
        putName("2.5.4.15", "businessCategory");
        putName("2.5.4.16", "postalAddress");
        putName("2.5.4.17", "postalCode");
        putName("2.5.4.18", "postOfficeBox");
        putName("2.5.4.20", "telephoneNumber", "TN");
        putName("2.5.4.41", "name", "N");
        putName("2.5.4.42", "givenName", "G");
        putName("2.5.4.43", "initials");
        putName("2.5.4.44", "generationQualifier");
        putName("2.5.4.45", "x500UniqueIdentifier");
        putName("2.5.4.46", "dnQualifier");
        putName("2.5.4.65", "pseudonym");
        putName("1.2.840.113549.1.9.1", "emailAddress");
        putName("1.2.840.113549.1.9.5", "signingTime");
        putName("1.3.6.1.5.5.7.9.1", "dateOfBirth");
        putName("1.3.6.1.5.5.7.9.2", "placeOfBirth");
        putName("1.3.6.1.5.5.7.9.3", "gender");
        putName("1.3.6.1.5.5.7.9.4", "countryOfCitizenship");
        putName("1.3.6.1.5.5.7.9.5", "countryOfResidence");
        putName("0.9.2342.19200300.100.1.25", "domainComponent", "DC");
        putName("0.9.2342.19200300.100.1.3", "rfc822Mailbox");
        putName("1.2.840.10040.4.1","dsaEncryption");
        putName("1.2.840.10040.4.3","dsaWithSHA1");
    }

}
