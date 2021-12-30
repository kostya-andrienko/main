#
# This file is the gpsd-clients recipe.
#

SUMMARY = "Simple gpsd-clients application"
SECTION = "PETALINUX/apps"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://cgps.elf \
		   file://ntpshmmon.elf \
		   file://gpsmon.elf \
		   file://gps2pl.elf \ 
		  "

S = "${WORKDIR}"

do_install() {
	     install -d ${D}${bindir}

	     install -m 0755 ${S}/cgps.elf ${D}/${bindir}
		 lnr ${D}/${bindir}/cgps.elf ${D}/${bindir}/cgps

		 install -m 0755 ${S}/ntpshmmon.elf ${D}/${bindir}
		 lnr ${D}/${bindir}/ntpshmmon.elf ${D}/${bindir}/ntpshmmon

		 install -m 0755 ${S}/gpsmon.elf ${D}/${bindir}
		 lnr ${D}/${bindir}/gpsmon.elf ${D}/${bindir}/gpsmon

         install -m 0755 ${S}/gps2pl.elf ${D}/${bindir}
		 lnr ${D}/${bindir}/gps2pl.elf ${D}/${bindir}/gps2pl
}

RDEPENDS_${PN} += "ncurses libgps libpython2"
