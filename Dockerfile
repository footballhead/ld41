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

#
# AS GAIA
#

USER gaia
WORKDIR /world
RUN mkdir -p stats
RUN mkdir -p rooms/town \
	&& echo "You find yourself in a small, peaceful town. The main streets are packed with smiling, happy people, just trying to make ends meet. The buildings are rustic yet charming. To the north you can see the city gates, the only way in and out of the recently walled-off space, and a meadow beyond." > rooms/town/description \
	&& mkdir -p rooms/meadow \
	&& echo "You're now in a quiet meadow, full of bright flowers saturated in the sunlight. There is a town to the south." > rooms/meadow/description \
	&& touch rooms/meadow/skeleton \
	&& chmod 666 rooms/meadow/skeleton \
	&& ln -s ../meadow rooms/town/north \
	&& ln -s ../town rooms/meadow/south

#
# AS ROOT
#

# Workaround for pseudoterminals ioctl failing
USER root
COPY ld41_entrypoint /usr/local/bin
ENTRYPOINT [ "/bin/sh", "-c", "ld41_entrypoint" ]
