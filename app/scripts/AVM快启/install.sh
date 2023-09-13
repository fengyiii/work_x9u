#!/bin/bash
echo -e "# /etc/systemd/system/tz_avm.service

[Unit]
Description=TZTEK AVM Service
After=network.target

[Service]
ExecStart=/bin/bash /home/root/avm_start.sh

[Install]
WantedBy=multi-user.target" > /etc/systemd/system/tz_avm.service


echo -e "#!/bin/bash
# /home/root/avm_start.sh

csi-test -d video-evs0 -g 0" > /home/root/avm_start.sh

systemctl enable tz_avm.service
chmod a+x /home/root/avm_start.sh
sync
