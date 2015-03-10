#!/bin/sh

TOOLS=$(dirname $0)

. ${TOOLS}/shell_utils

check_vars ROOT_PROJ

SOURCE_DIR=${ROOT_PROJ}/sources

check_dirs $ROOT_PROJ $SOURCE_DIR

SVN_BASE=https://fre-svn.corp.avocent.com/svn/avocent-gsp/sources/generic
branch=trunk

PGM=$(basename ${0})

function show_usage()
{
  echo
  if [ -n "${1}" ] ; then
    echo "ERROR:" ${1}
    echo
  fi
  echo "USAGE: ${PGM} [--force] [-b <branch>]"
  echo
  exit 1
}

function update_dir()
{
  pushd ${1} >/dev/null
  echo "SVN updating: ${1}"
  echo "From: " $(svn info | grep "URL:")
  svn up
  popd >/dev/null
}

function checkout_dir()
{
  SVN_URL=${SVN_BASE}/${2}/${branch}/${3}
  echo "Checking out: ${SVN_URL}"
  echo "Into: ${1}"
  svn co ${SVN_URL} ${1}
}

while [ "x${1}" != "x" ] ; do
  case ${1} in
  --force)
    rm -rf pmd-gsp/gspsrc
    rm -rf pmclib/gsp-include
  ;;
  -b)
    if [ -n "${2}" ] ; then
      branch=branches/${2}
      shift
    else
      show_usage "Branch name must be given."
    fi
  ;;
  *)
    show_usage
  ;;
  esac
  shift
done

cd ${SOURCE_DIR} || exit 1

if [ -d pmd-gsp/gspsrc ] ; then
  update_dir pmd-gsp/gspsrc
else
  checkout_dir pmd-gsp/gspsrc power-mgmt-2.0.0 src
fi

if [ -d pmclib/gsp-include ] ; then
  update_dir pmclib/gsp-include
else
  checkout_dir pmclib/gsp-include fm-power-mgmt-2.0.0 include
fi
