# $Id: Makefile,v 1.1 2021-02-17 18:21:46-08 - - $

ALL     := ${basename ${filter %.mm, ${shell ls -t}}}
NEW     := ${firstword ${ALL}}

GROFFDIR = /afs/cats.ucsc.edu/courses/cse110a-wm/groff
DEPS     = ${GROFFDIR}/tmac/Tmac.mm-etc Makefile
DIROPTS  = -F${GROFFDIR}/font -I${GROFFDIR}/tmac -M${GROFFDIR}/tmac
GROFFMM  = groff -mgm -U -b -ww ${DIROPTS}
PSOPTS   = -spte -Tps
TTOPTS   = -st -Tlatin1 -P-cbuo
SQUEEZE  = perl -00pe0
STARS    = ************************************************************
LABEL    = echo "${STARS}"; echo $@: $< ${1}; echo "${STARS}"

newest : ${NEW}.tt ${NEW}.pdf
	- pkill -s 0 gv || true
	gv ${NEW}.ps &

all : tt ps pdf

ps : ${ALL:%=%.ps}
tt : ${ALL:%=%.tt}
pdf : ${ALL:%=%.pdf}

% : %.pdf

%.ps : %.mm ${DEPS}
	@ ${call LABEL, ${PSOPTS}}
	cid -is $<
	${GROFFMM} ${PSOPTS} -z -rRef=1 $<
	${GROFFMM} ${PSOPTS} $< >$@
	letterbbox $@

%.tt : %.mm ${DEPS}
	@ ${call LABEL, ${TTOPTS}}
	cid -is $<
	${GROFFMM} ${TTOPTS} -z -rRef=1 $<
	${GROFFMM} ${TTOPTS} $< | ${SQUEEZE} >$@

%.pdf : %.ps
	mkpdf $<

