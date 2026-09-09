# ESP32 System Monitor - PC Client (WMI GPU Performance Counters)
# Uses same data source as Windows Task Manager

# Silent
$host.UI.RawUI.WindowTitle = "ESP32 Monitor (Running)"
$ErrorActionPreference = "SilentlyContinue"
[Console]::CursorVisible = $false

# --- CONFIGURATION ---
$ESP32_HOSTNAME = "esp32monitor.local"
$UDP_PORT = 4210

# Resolve mDNS to IP
try {
    Write-Host "=== ESP32 Monitor Client ===" -ForegroundColor Green
    Write-Host "Resolving $ESP32_HOSTNAME..." -ForegroundColor Cyan
    $ips = [System.Net.Dns]::GetHostAddresses($ESP32_HOSTNAME)
    $ESP32_IP = $ips[0].IPAddressToString
    Write-Host "Resolved to: $ESP32_IP" -ForegroundColor Green
} catch {
    Write-Host "ERROR: Cannot resolve $ESP32_HOSTNAME" -ForegroundColor Red
    Write-Host "Make sure ESP32 is connected to the same network." -ForegroundColor Yellow
    exit 1
}

# Initialize UDP client
$udpClient = New-Object System.Net.Sockets.UdpClient
$udpClient.Connect($ESP32_IP, $UDP_PORT)

Write-Host "UDP Port: $UDP_PORT" -ForegroundColor Cyan
Write-Host "Using WMI GPU Performance Counters (Task Manager method)" -ForegroundColor Cyan
Write-Host "Press Ctrl+C to stop" -ForegroundColor Yellow
Write-Host ""

# Initialize CPU counter
$cpuCounter = New-Object System.Diagnostics.PerformanceCounter("Processor", "% Processor Time", "_Total")
$null = $cpuCounter.NextValue()
Start-Sleep -Milliseconds 500

# RAM counter
$ramCounter = New-Object System.Diagnostics.PerformanceCounter("Memory", "% Committed Bytes In Use")

try {
    while ($true) {
        # 1. CPU Usage (%)
        $cpuUsage = [math]::Round($cpuCounter.NextValue())

        # 2. RAM Usage (%)
        $ramUsage = [math]::Round($ramCounter.NextValue())

        # 3. GPU Usage (%) - via WMI (same as Task Manager)
        $gpuUsage = 0
        try {
            # Get GPU usage from WMI (3D usage like in Task Manager)
            $gpuData = Get-CimInstance Win32_PerfFormattedData_GPUPerformanceCounters_GPUEngine -ErrorAction SilentlyContinue | 
                Where-Object { $_.Name -like "*3D*" -or $_.Name -like "*enginerender*" } |
                Measure-Object -Property UtilizationPercentage -Sum |
                Select-Object -ExpandProperty Sum
            
            if ($gpuData) {
                $gpuUsage = [math]::Round($gpuData)
            }
        } catch {
            $gpuUsage = 0
        }

        # 4. Temperatures (estimated based on usage)
        if ($cpuUsage -gt 80) {
            $cpuTemp = 65.0 + (Get-Random -Maximum 15)
        } elseif ($cpuUsage -gt 50) {
            $cpuTemp = 50.0 + (Get-Random -Maximum 10)
        } else {
            $cpuTemp = 40.0 + (Get-Random -Maximum 5)
        }
        
        if ($gpuUsage -gt 80) {
            $gpuTemp = 60.0 + (Get-Random -Maximum 10)
        } elseif ($gpuUsage -gt 30) {
            $gpuTemp = 50.0 + (Get-Random -Maximum 8)
        } else {
            $gpuTemp = 45.0 + (Get-Random -Maximum 5)
        }

        # Format JSON
        $payload = @{
            cpu_temp  = [math]::Round($cpuTemp, 1)
            gpu_temp  = [math]::Round($gpuTemp, 1)
            cpu_usage = $cpuUsage
            gpu_usage = $gpuUsage
            ram_usage = $ramUsage
        } | ConvertTo-Json -Compress

        # Send UDP packet
        $bytes = [System.Text.Encoding]::UTF8.GetBytes($payload)
        $udpClient.Send($bytes, $bytes.Length) | Out-Null

        # Console output
        Write-Host "CPU: $cpuUsage% $([math]::Round($cpuTemp))C | GPU: $gpuUsage% $([math]::Round($gpuTemp))C | RAM: $ramUsage%" -ForegroundColor Green

        Start-Sleep -Milliseconds 500
    }
}
finally {
    $udpClient.Close()
    Write-Host ""
    Write-Host "Monitoring stopped." -ForegroundColor Red
}