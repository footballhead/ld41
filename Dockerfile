FROM ubuntu:latest

COPY motd /etc/motd

# https://medium.com/@mccode/understanding-how-uid-and-gid-work-in-docker-containers-c37a01d01cf
RUN useradd -r -u 1001 ld41 \
	&& mkdir -p /home/ld41 \
	&& chown ld41:ld41 /home/ld41

WORKDIR /home/ld41
COPY .bashrc .bashrc

USER ld41
ENTRYPOINT [ "bash" ]
