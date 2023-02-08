# SeC-OMC-Coder

[![Build-macOS-latest Actions Status](https://github.com/rozhuk-im/sec-omc-coder/workflows/build-macos-latest/badge.svg)](https://github.com/rozhuk-im/sec-omc-coder/actions)
[![Build-Ubuntu-latest Actions Status](https://github.com/rozhuk-im/sec-omc-coder/workflows/build-ubuntu-latest/badge.svg)](https://github.com/rozhuk-im/sec-omc-coder/actions)


Rozhuk Ivan <rozhuk.im@gmail.com> 2021

Decode / Encode Samsung CSC files.\
Based on: https://github.com/fei-ke/OmcTextDecoder


## Licence
BSD licence.


## Compilation

### Linux
``` shell
sudo apt-get install build-essential git fakeroot
git clone https://github.com/rozhuk-im/sec-omc-coder.git
cd sec-omc-coder/src
cc sec-omc-coder.c -O2 -lm -o sec-omc-coder
```

### FreeBSD/DragonFlyBSD
``` shell
sudo pkg install git
git clone https://github.com/rozhuk-im/sec-omc-coder.git
cd sec-omc-coder/src
cc sec-omc-coder.c -O2 -lm -o sec-omc-coder
```


## Usage

### Decode
``` shell
cat cscfeature.xml | sec-omc-coder -d | gzip --decompress
```
or
``` shell
sec-omc-coder -d cscfeature.xml | gzip --decompress
```

### Encode
``` shell
cat cscfeature.xml | gzip --best | sec-omc-coder -e
```

