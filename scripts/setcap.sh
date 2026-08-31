#!/bin/sh

sudo setcap cap_net_raw=ep ./ft_ping && getcap ./ft_ping