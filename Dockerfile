FROM ubuntu:latest

RUN mkdir -p /etc/skel/stats \
	&& echo 18 > /etc/skel/stats/strength \
	&& echo 18 > /etc/skel/stats/dexterity \
	&& echo 18 > /etc/skel/stats/magic \
	&& echo 18 > /etc/skel/stats/vitality

RUN chmod -x /etc/update-motd.d/00-header /etc/update-motd.d/10-help-text \
	&& mv /etc/legal /etc/legal_
COPY motd /etc/motd

ARG LOGIN_USER=adventurer
# openssl passwd -crypt ld41
ARG LOGIN_PASS=c9tHOGXsa3ES.
RUN useradd ${LOGIN_USER} -d /home/${LOGIN_USER} -m -p ${LOGIN_PASS}

ENTRYPOINT [ "/bin/sh", "-c", "login" ]
