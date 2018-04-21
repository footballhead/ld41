FROM ubuntu:latest

# Add stats files to skeleton directory
WORKDIR /etc/skel
RUN mkdir -p stats \
	&& echo 18 > stats/strength \
	&& echo 18 > stats/dexterity \
	&& echo 18 > stats/magic \
	&& echo 18 > stats/vitality
RUN mkdir -p town \
	&& echo "You find yourself in a small, peaceful town. The main streets are packed with smiling, happy people, just trying to make ends meet. The buildings are rustic yet charming. To the north you can see the city gates, the only way in and out of the recently walled-off space, and a meadow beyond." > town/description \
	&& mkdir -p town/north \
	&& echo "You're now in a quiet meadow, full of gently bright flowers saturated in sunlight. There is a town to the south." > town/north/description \
	&& ln -s .. town/north/south

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
