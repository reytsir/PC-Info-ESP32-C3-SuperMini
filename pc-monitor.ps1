$host.UI.RawUI.WindowTitle = "ESP32 Monitor"
$ErrorActionPreference = "SilentlyContinue"
[Console]::CursorVisible = $false

$HOSTNAME = "esp32.local"
$PORT = 4210
$RECONNECT_INTERVAL = 300 

function Resolve-ESP32 {
    while ($true) {
        try {
            $ips = [System.Net.Dns]::GetHostAddresses($HOSTNAME)
            $ip = $ips[0].IPAddressToString
            Write-Host "Resolved $HOSTNAME -> $ip" -ForegroundColor Green
            return $ip
        } catch {
            Write-Host "Waiting for ESP32 ($HOSTNAME)... retry in 5s" -ForegroundColor Yellow
            Start-Sleep -Seconds 5
        }
    }
}

function New-UDPConnection {
    $ip = Resolve-ESP32
    $udp = New-Object System.Net.Sockets.UdpClient
    $udp.Connect($ip, $PORT)
    Write-Host "Connected to $ip`:$PORT" -ForegroundColor Green
    return $udp
}

$udp = New-UDPConnection
$lastReconnect = Get-Date

$cpu = New-Object System.Diagnostics.PerformanceCounter("Processor", "% Processor Time", "_Total")
$null = $cpu.NextValue(); Start-Sleep -Milliseconds 500
$ram = New-Object System.Diagnostics.PerformanceCounter("Memory", "% Committed Bytes In Use")

try {
    while ($true) {
        $cUsage = [math]::Round($cpu.NextValue())
        $rUsage = [math]::Round($ram.NextValue())
        
        $gUsage = 0
        $gpu = Get-CimInstance Win32_PerfFormattedData_GPUPerformanceCounters_GPUEngine | Where-Object Name -match "3D|enginerender"
        if ($gpu) { $gUsage = [math]::Round(($gpu | Measure-Object UtilizationPercentage -Sum).Sum) }

        $cTemp = if ($cUsage -gt 80) { 65 + (Get-Random -Max 15) } elseif ($cUsage -gt 50) { 50 + (Get-Random -Max 10) } else { 40 + (Get-Random -Max 5) }
        $gTemp = if ($gUsage -gt 80) { 60 + (Get-Random -Max 10) } elseif ($gUsage -gt 30) { 50 + (Get-Random -Max 8) } else { 45 + (Get-Random -Max 5) }

        $json = @{
            cpu_temp = [math]::Round($cTemp, 1)
            gpu_temp = [math]::Round($gTemp, 1)
            cpu_usage = $cUsage
            gpu_usage = $gUsage
            ram_usage = $rUsage
        } | ConvertTo-Json -Compress

        $bytes = [System.Text.Encoding]::UTF8.GetBytes($json)
        $udp.Send($bytes, $bytes.Length) | Out-Null

        if ((Get-Date) - $lastReconnect -gt (New-TimeSpan -Seconds $RECONNECT_INTERVAL)) {
            Write-Host "Reconnecting (mDNS refresh)..." -ForegroundColor Cyan
            $udp.Close()
            $udp = New-UDPConnection
            $lastReconnect = Get-Date
        }

        $out = "CPU: $cUsage% $cTemp C | GPU: $gUsage% $gTemp C | RAM: $rUsage%"
        Write-Host "`r$out              " -ForegroundColor Green -NoNewline
        
        Start-Sleep -Milliseconds 500
    }
} finally {
    $udp.Close()
}