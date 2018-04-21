FROM ubuntu:latest

COPY motd /etc/motd
COPY 00_motd /etc/profile.d/00_motd
RUN chmod +x /etc/profile.d/00_motd

# https://medium.com/@mccode/understanding-how-uid-and-gid-work-in-docker-containers-c37a01d01cf
ARG LOGIN_USER=adventurer
ARG LOGIN_DIR=/home/${LOGIN_USER}
RUN useradd -r -u 1001 ${LOGIN_USER} -d ${LOGIN_DIR}\
	&& mkdir -p ${LOGIN_DIR} \
	&& chown ${LOGIN_USER}:${LOGIN_USER} ${LOGIN_DIR}

# --chown doesn't like using variables, so RUN chown instead
#COPY .bashrc ${LOGIN_DIR}/.bashrc
#RUN chown ${LOGIN_USER}:${LOGIN_USER} ${LOGIN_DIR}/.bashrc

RUN mkdir -p ${LOGIN_DIR}/stats \
	&& echo 18 > ${LOGIN_DIR}/stats/strength \
	&& echo 18 > ${LOGIN_DIR}/stats/dexterity \
	&& echo 18 > ${LOGIN_DIR}/stats/magic \
	&& echo 18 > ${LOGIN_DIR}/stats/vitality

WORKDIR ${LOGIN_DIR}
ENTRYPOINT [ "login" ]
