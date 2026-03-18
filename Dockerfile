FROM alpine as base 
RUN apk add --no-cache clang make musl-dev gtest-dev sqlite-dev uriparser-dev openssl-dev
WORKDIR /usr/src/ripple
CMD [ "sh" ]