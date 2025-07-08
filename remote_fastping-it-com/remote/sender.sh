mkdir -p ~/vps_grab && \
nohup rsync -avzP \
  -e "ssh -p 3456" \
  --log-file=~/vps_grab/rsync-transfer.log \
  wofl@138.197.11.134:/home/wofl/proxy_test_app/ \
  ~/WOFLAPTOP/D/online_fastping-it-com/ \
  && notify-send "Transfer Complete" "VPS download finished" \
  > /dev/null 2>&1 &
