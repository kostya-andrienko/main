#
# This file is the misc recipe.
#

SUMMARY = "Simple misc application"
SECTION = "PETALINUX/apps"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://ser2net.conf \
		   file://statuswatch  \
		   file://codate \
		  "

S = "${WORKDIR}"

do_install() {
	    install -d ${D}/etc
	    install -m 0755 ${S}/ser2net.conf ${D}/etc

		install -d ${D}/usr/sbin
		install -m 0755 ${S}/statuswatch ${D}/usr/sbin	

		install -d ${D}/usr/sbin
		install -m 0755 ${S}/codate ${D}/usr/sbin
}

RDEPENDS_${PN} += "bash"
