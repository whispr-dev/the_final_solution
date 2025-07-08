$udpClient = New-Object System.Net.Sockets.UdpClient(9876)
$endpoint = New-Object System.Net.IPEndPoint ([System.Net.IPAddress]::Any, 0)
while ($true) {
    $data = $udpClient.Receive([ref]$endpoint)
    $message = [System.Text.Encoding]::ASCII.GetString($data)
    Write-Output "[$([datetime]::Now.ToString("HH:mm:ss"))] From $($endpoint.Address): $message"
}
