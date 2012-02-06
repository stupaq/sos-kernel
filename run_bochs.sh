#!/bin/bash

bochs -qf /dev/null 'megs: 64' 'floppya: 1_44=floppy.img, status=inserted' 'boot: floppy' 'log: bochs.log' 'clock: sync=realtime' 'cpu: ips=200000'

