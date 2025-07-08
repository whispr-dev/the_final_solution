# Reset the failure counter (wake up!)
sudo systemctl reset-failed proxy-test

# Then start it again
sudo systemctl start proxy-test