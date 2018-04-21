FROM ubuntu:latest

ARG LOGIN_USER=adventurer
# openssl passwd -crypt ld41
ARG LOGIN_PASS=c9tHOGXsa3ES.
ARG LOGIN_DIR=/home/${LOGIN_USER}
RUN useradd ${LOGIN_USER} -d ${LOGIN_DIR} -m -p ${LOGIN_PASS}

# RUN mkdir -p ${LOGIN_DIR}/stats \
# 	&& echo 18 > ${LOGIN_DIR}/stats/strength \
# 	&& echo 18 > ${LOGIN_DIR}/stats/dexterity \
# 	&& echo 18 > ${LOGIN_DIR}/stats/magic \
# 	&& echo 18 > ${LOGIN_DIR}/stats/vitality

ENTRYPOINT [ "login" ]
