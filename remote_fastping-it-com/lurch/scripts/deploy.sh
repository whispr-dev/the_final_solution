#!/bin/bash
# VPS Flask Server Complete Setup Script
# Run as: chmod +x setup_vps.sh && ./setup_vps.sh

echo "🚀 Setting up Complete Flask Production Environment..."

# Update system
sudo apt update && sudo apt upgrade -y

# Install system dependencies
sudo apt install -y \
    python3 \
    python3-pip \
    python3-venv \
    python3-dev \
    nginx \
    postgresql \
    postgresql-contrib \
    redis-server \
    supervisor \
    ufw \
    certbot \
    python3-certbot-nginx \
    build-essential \
    curl \
    git \
    htop \
    fail2ban

echo "📦 Installing Python dependencies..."

# Create virtual environment
python3 -m venv venv
source venv/bin/activate

# Upgrade pip
pip install --upgrade pip setuptools wheel

# Install all requirements
pip install -r requirements.txt

echo "⚙️ Configuring Nginx..."

# Nginx configuration
sudo tee /etc/nginx/sites-available/flask_app << 'EOF'
server {
    listen 80;
    server_name your_domain.com www.your_domain.com;
    
    # Security headers
    add_header X-Frame-Options "SAMEORIGIN" always;
    add_header X-XSS-Protection "1; mode=block" always;
    add_header X-Content-Type-Options "nosniff" always;
    add_header Referrer-Policy "no-referrer-when-downgrade" always;
    add_header Content-Security-Policy "default-src 'self' http: https: data: blob: 'unsafe-inline'" always;
    
    # Gzip compression
    gzip on;
    gzip_vary on;
    gzip_min_length 1024;
    gzip_proxied expired no-cache no-store private must-revalidate auth;
    gzip_types text/plain text/css text/xml text/javascript application/x-javascript application/json application/xml+rss;
    
    location / {
        proxy_pass http://127.0.0.1:8000;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
        
        # Timeout settings
        proxy_connect_timeout 30s;
        proxy_send_timeout 30s;
        proxy_read_timeout 30s;
        
        # Buffer settings
        proxy_buffering on;
        proxy_buffer_size 128k;
        proxy_buffers 4 256k;
        proxy_busy_buffers_size 256k;
    }
    
    # Static files
    location /static {
        alias /var/www/flask_app/static;
        expires 1y;
        add_header Cache-Control "public, immutable";
    }
    
    # Health check
    location /health {
        access_log off;
        return 200 "healthy\n";
        add_header Content-Type text/plain;
    }
}
EOF

# Enable site
sudo ln -sf /etc/nginx/sites-available/flask_app /etc/nginx/sites-enabled/
sudo rm -f /etc/nginx/sites-enabled/default

echo "🔧 Configuring Gunicorn..."

# Gunicorn configuration
sudo tee /etc/systemd/system/flask_app.service << 'EOF'
[Unit]
Description=Gunicorn instance to serve Flask App
After=network.target

[Service]
User=www-data
Group=www-data
WorkingDirectory=/var/www/flask_app
Environment="PATH=/var/www/flask_app/venv/bin"
ExecStart=/var/www/flask_app/venv/bin/gunicorn --workers 4 --bind 127.0.0.1:8000 --timeout 30 --keep-alive 2 --max-requests 1000 --max-requests-jitter 50 app:app
Restart=always

[Install]
WantedBy=multi-user.target
EOF

echo "🛡️ Configuring Security..."

# Firewall setup
sudo ufw --force enable
sudo ufw default deny incoming
sudo ufw default allow outgoing
sudo ufw allow ssh
sudo ufw allow 'Nginx Full'

# Fail2ban configuration for nginx
sudo tee /etc/fail2ban/jail.local << 'EOF'
[nginx-http-auth]
enabled = true
port = http,https
logpath = /var/log/nginx/error.log

[nginx-noscript]
enabled = true
port = http,https
logpath = /var/log/nginx/access.log
maxretry = 6

[nginx-badbots]
enabled = true
port = http,https
logpath = /var/log/nginx/access.log
maxretry = 2

[nginx-noproxy]
enabled = true
port = http,https
logpath = /var/log/nginx/access.log
maxretry = 2
EOF

echo "📊 Setting up monitoring..."

# Supervisor configuration for monitoring
sudo tee /etc/supervisor/conf.d/flask_app.conf << 'EOF'
[program:flask_app]
command=/var/www/flask_app/venv/bin/gunicorn --workers 4 --bind 127.0.0.1:8000 app:app
directory=/var/www/flask_app
user=www-data
autostart=true
autorestart=true
redirect_stderr=true
stdout_logfile=/var/log/flask_app.log
EOF

echo "🗄️ Setting up databases..."

# PostgreSQL setup
sudo -u postgres createuser --interactive --pwprompt flask_user
sudo -u postgres createdb flask_db --owner=flask_user

# Redis configuration
sudo systemctl enable redis-server
sudo systemctl start redis-server

echo "📝 Creating environment file..."

# Environment configuration
tee .env << 'EOF'
# Flask Configuration
FLASK_APP=app.py
FLASK_ENV=production
SECRET_KEY=your_super_secret_key_here
DEBUG=False

# Database
DATABASE_URL=postgresql://flask_user:password@localhost/flask_db
REDIS_URL=redis://localhost:6379/0

# Security
WTF_CSRF_ENABLED=True
SESSION_COOKIE_SECURE=True
SESSION_COOKIE_HTTPONLY=True

# Mail (if using)
MAIL_SERVER=smtp.gmail.com
MAIL_PORT=587
MAIL_USE_TLS=True
MAIL_USERNAME=your_email@gmail.com
MAIL_PASSWORD=your_app_password

# Logging
LOG_LEVEL=INFO
LOG_FILE=/var/log/flask_app.log
EOF

echo "🔄 Starting services..."

# Set permissions
sudo chown -R www-data:www-data /var/www/flask_app
sudo chmod -R 755 /var/www/flask_app

# Start services
sudo systemctl daemon-reload
sudo systemctl enable nginx
sudo systemctl enable flask_app
sudo systemctl start nginx
sudo systemctl start flask_app
sudo systemctl enable fail2ban
sudo systemctl start fail2ban
sudo supervisorctl reread
sudo supervisorctl update

echo "✅ Flask production environment setup complete!"
echo ""
echo "🔧 Next steps:"
echo "1. Update .env with your actual values"
echo "2. Update nginx config with your domain"
echo "3. Run: sudo certbot --nginx -d your_domain.com"
echo "4. Test: curl http://your_domain.com/health"
echo ""
echo "📊 Monitor with:"
echo "- sudo systemctl status flask_app"
echo "- sudo systemctl status nginx"
echo "- sudo tail -f /var/log/flask_app.log"
echo "- sudo nginx -t (test config)"
echo ""
echo "🚀 Your Flask app is ready for production!"