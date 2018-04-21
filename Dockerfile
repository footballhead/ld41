FROM ubuntu:latest

# Add stats files to skeleton directory
RUN mkdir -p /etc/skel/stats \
	&& echo 18 > /etc/skel/stats/strength \
	&& echo 18 > /etc/skel/stats/dexterity \
	&& echo 18 > /etc/skel/stats/magic \
	&& echo 18 > /etc/skel/stats/vitality

# Disable default MOTD and legal message, then copy our own
RUN chmod -x /etc/update-motd.d/00-header /etc/update-motd.d/10-help-text \
	&& mv /etc/legal /etc/legal_
COPY motd /etc/motd

# Add the new user. Password made with: openssl passwd -crypt ld41
ARG LOGIN_USER=adventurer
ARG LOGIN_PASS=c9tHOGXsa3ES.
RUN useradd ${LOGIN_USER} -d /home/${LOGIN_USER} -m -p ${LOGIN_PASS}

# Workaround for pseudoterminals ioctl failing
ENTRYPOINT [ "/bin/sh", "-c", "login" ]
