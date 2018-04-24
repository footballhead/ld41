#
# IMAGE: build
# RESPONSIBILITY: Build all required source code
#

FROM ubuntu:latest as build

RUN apt-get update \
	&& apt-get install -y \
		make \
		gcc

WORKDIR /build
COPY src .
RUN make

#
# IMAGE: setup
# RESPONSIBILITY: Create the container that the player will run around in
#

FROM ubuntu:latest as setup

#
# AS ROOT
#

# Disable default MOTD and legal message, then copy our own
RUN chmod -x /etc/update-motd.d/00-header /etc/update-motd.d/10-help-text \
	&& mv /etc/legal /etc/legal_
COPY motd /etc/motd

# HACK: Allow other users to inject things into the player terminal
RUN echo "chmod 666 /dev/pts/0" >> /etc/skel/.profile

# Game admin, owner of game files
RUN useradd -r gaia -d /world -m -G tty

# Add the new user. Password made with: openssl passwd -crypt ld41
ARG LOGIN_USER=adventurer
ARG LOGIN_PASS=c9tHOGXsa3ES.
RUN useradd ${LOGIN_USER} -d /world -p ${LOGIN_PASS}

COPY --from=build /build/monsterd /usr/local/bin/monsterd
COPY --from=build /build/rpgstatsd /usr/local/bin/rpgstatsd
COPY --from=build /build/riddlerd /usr/local/bin/riddlerd
COPY deathwatch /usr/local/bin/deathwatch

COPY rooms /world
RUN chown -R gaia:gaia /world/starter_town

#
# AS GAIA
#

USER gaia
WORKDIR /world
RUN mkdir -p stats



#
# AS ROOT
#

# Workaround for pseudoterminals ioctl failing
USER root
COPY ld41_entrypoint /usr/local/bin
ENTRYPOINT [ "/bin/sh", "-c", "ld41_entrypoint" ]
