"""
Automated Customer Onboarding System - Continuation
=================================================

Completing the onboarding system with Flask integration and monitoring
"""

    def get_onboarding_status(self, email: str) -> Dict:
        """Get onboarding status for a customer"""
        conn = sqlite3.connect('customer_resources.db')
        cursor = conn.cursor()
        
        cursor.execute('''
            SELECT event_id, onboarding_status, created_at, completed_at, error_message
            FROM onboarding_events 
            WHERE customer_email = ? 
            ORDER BY created_at DESC 
            LIMIT 1
        ''', (email,))
        
        event_result = cursor.fetchone()
        
        if not event_result:
            conn.close()
            return {'status': 'not_found'}
        
        event_id, status, created_at, completed_at, error_message = event_result
        
        # Get step details
        cursor.execute('''
            SELECT step_name, step_status, started_at, completed_at, error_details
            FROM onboarding_steps 
            WHERE event_id = ? 
            ORDER BY started_at
        ''', (event_id,))
        
        steps = []
        for row in cursor.fetchall():
            step_name, step_status, started_at, step_completed_at, error_details = row
            steps.append({
                'step_name': step_name,
                'status': step_status,
                'started_at': started_at,
                'completed_at': step_completed_at,
                'error_details': error_details
            })
        
        conn.close()
        
        return {
            'status': status,
            'created_at': created_at,
            'completed_at': completed_at,
            'error_message': error_message,
            'steps': steps
        }
    
    def retry_failed_onboarding(self, email: str) -> bool:
        """Manually retry failed onboarding"""
        try:
            conn = sqlite3.connect('customer_resources.db')
            cursor = conn.cursor()
            
            # Get latest failed onboarding
            cursor.execute('''
                SELECT event_id, customer_email, plan_type, payment_amount, paypal_transaction_id
                FROM onboarding_events 
                WHERE customer_email = ? AND onboarding_status = 'failed'
                ORDER BY created_at DESC 
                LIMIT 1
            ''', (email,))
            
            result = cursor.fetchone()
            conn.close()
            
            if not result:
                return False
            
            event_id, customer_email, plan_type, payment_amount, paypal_transaction_id = result
            
            # Create new onboarding event for retry
            retry_event = OnboardingEvent(
                event_id=str(uuid.uuid4()),
                customer_email=customer_email,
                plan_type=plan_type,
                payment_amount=payment_amount,
                paypal_transaction_id=f"retry_{paypal_transaction_id}",
                timestamp=datetime.now()
            )
            
            # Start retry process
            threading.Thread(
                target=self._execute_onboarding_flow, 
                args=(retry_event,),
                daemon=True
            ).start()
            
            return True
            
        except Exception as e:
            logger.error(f"Failed to retry onboarding for {email}: {e}")
            return False

# Flask Integration for Automated Onboarding
def integrate_automated_onboarding(app, customer_manager, whitelist_manager, paypal_manager):
    """Integrate automated onboarding with Flask app"""
    
    onboarding_manager = AutomatedOnboardingManager(
        paypal_manager, customer_manager, whitelist_manager
    )
    
    @app.route('/webhook/paypal/onboarding', methods=['POST'])
    def paypal_onboarding_webhook():
        """Enhanced PayPal webhook handler for onboarding"""
        try:
            webhook_data = request.get_json()
            headers = dict(request.headers)
            
            # Verify webhook signature (production security)
            if not onboarding_manager._verify_webhook_signature(webhook_data, headers):
                logger.warning("Invalid webhook signature")
                return jsonify({'error': 'Invalid signature'}), 401
            
            # Process webhook and trigger onboarding
            success = onboarding_manager.process_paypal_webhook(webhook_data)
            
            if success:
                return jsonify({'status': 'processed'}), 200
            else:
                return jsonify({'status': 'error'}), 400
                
        except Exception as e:
            logger.error(f"Webhook processing error: {e}")
            return jsonify({'error': 'Processing failed'}), 500
    
    @app.route('/api/onboarding/status/<email>')
    def get_onboarding_status_api(email):
        """Get onboarding status via API"""
        try:
            status = onboarding_manager.get_onboarding_status(email)
            return jsonify(status)
        except Exception as e:
            logger.error(f"Error getting onboarding status: {e}")
            return jsonify({'error': 'Failed to get status'}), 500
    
    @app.route('/api/onboarding/retry', methods=['POST'])
    def retry_onboarding_api():
        """Manually retry failed onboarding"""
        try:
            data = request.get_json()
            email = data.get('email')
            
            if not email:
                return jsonify({'error': 'Email required'}), 400
            
            success = onboarding_manager.retry_failed_onboarding(email)
            
            if success:
                return jsonify({'success': True, 'message': 'Retry initiated'})
            else:
                return jsonify({'success': False, 'message': 'No failed onboarding found'}), 404
                
        except Exception as e:
            logger.error(f"Error retrying onboarding: {e}")
            return jsonify({'error': 'Retry failed'}), 500
    
    @app.route('/admin/onboarding/dashboard')
    def onboarding_dashboard():
        """Admin dashboard for onboarding monitoring"""
        try:
            conn = sqlite3.connect('customer_resources.db')
            cursor = conn.cursor()
            
            # Get recent onboarding events
            cursor.execute('''
                SELECT customer_email, plan_type, onboarding_status, created_at, completed_at
                FROM onboarding_events 
                ORDER BY created_at DESC 
                LIMIT 50
            ''')
            
            events = []
            for row in cursor.fetchall():
                email, plan_type, status, created_at, completed_at = row
                events.append({
                    'email': email,
                    'plan_type': plan_type,
                    'status': status,
                    'created_at': created_at,
                    'completed_at': completed_at
                })
            
            # Get summary stats
            cursor.execute('''
                SELECT 
                    COUNT(*) as total,
                    SUM(CASE WHEN onboarding_status = 'completed' THEN 1 ELSE 0 END) as completed,
                    SUM(CASE WHEN onboarding_status = 'failed' THEN 1 ELSE 0 END) as failed,
                    SUM(CASE WHEN onboarding_status = 'pending' THEN 1 ELSE 0 END) as pending
                FROM onboarding_events 
                WHERE created_at >= date('now', '-7 days')
            ''')
            
            stats = cursor.fetchone()
            conn.close()
            
            dashboard_html = f'''
            <!DOCTYPE html>
            <html>
            <head>
                <title>Onboarding Dashboard</title>
                <style>
                    body {{ font-family: Arial, sans-serif; margin: 20px; background: #f5f5f5; }}
                    .container {{ max-width: 1200px; margin: 0 auto; }}
                    .stats {{ display: grid; grid-template-columns: repeat(4, 1fr); gap: 20px; margin-bottom: 30px; }}
                    .stat-card {{ background: white; padding: 20px; border-radius: 8px; text-align: center; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }}
                    .stat-number {{ font-size: 2rem; font-weight: bold; color: #40e0ff; }}
                    .stat-label {{ color: #666; margin-top: 5px; }}
                    .events-table {{ background: white; border-radius: 8px; overflow: hidden; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }}
                    table {{ width: 100%; border-collapse: collapse; }}
                    th, td {{ padding: 12px; text-align: left; border-bottom: 1px solid #eee; }}
                    th {{ background: #f8f9fa; font-weight: 600; }}
                    .status-completed {{ color: #28a745; font-weight: bold; }}
                    .status-failed {{ color: #dc3545; font-weight: bold; }}
                    .status-pending {{ color: #ffc107; font-weight: bold; }}
                    .refresh-btn {{ background: #40e0ff; color: white; padding: 10px 20px; border: none; border-radius: 5px; cursor: pointer; }}
                </style>
            </head>
            <body>
                <div class="container">
                    <h1>🚀 Onboarding Dashboard</h1>
                    
                    <div class="stats">
                        <div class="stat-card">
                            <div class="stat-number">{stats[0]}</div>
                            <div class="stat-label">Total (7 days)</div>
                        </div>
                        <div class="stat-card">
                            <div class="stat-number">{stats[1]}</div>
                            <div class="stat-label">Completed</div>
                        </div>
                        <div class="stat-card">
                            <div class="stat-number">{stats[2]}</div>
                            <div class="stat-label">Failed</div>
                        </div>
                        <div class="stat-card">
                            <div class="stat-number">{stats[3]}</div>
                            <div class="stat-label">Pending</div>
                        </div>
                    </div>
                    
                    <div class="events-table">
                        <table>
                            <thead>
                                <tr>
                                    <th>Email</th>
                                    <th>Plan</th>
                                    <th>Status</th>
                                    <th>Started</th>
                                    <th>Completed</th>
                                    <th>Actions</th>
                                </tr>
                            </thead>
                            <tbody>
            '''
            
            for event in events:
                status_class = f"status-{event['status']}"
                completed_display = event['completed_at'] or '-'
                
                dashboard_html += f'''
                                <tr>
                                    <td>{event['email']}</td>
                                    <td>{event['plan_type'].title()}</td>
                                    <td class="{status_class}">{event['status'].title()}</td>
                                    <td>{event['created_at'][:19]}</td>
                                    <td>{completed_display}</td>
                                    <td>
                                        <button onclick="retryOnboarding('{event['email']}')" 
                                                class="refresh-btn">Retry</button>
                                    </td>
                                </tr>
                '''
            
            dashboard_html += '''
                            </tbody>
                        </table>
                    </div>
                </div>
                
                <script>
                    function retryOnboarding(email) {
                        if (confirm(`Retry onboarding for ${email}?`)) {
                            fetch('/api/onboarding/retry', {
                                method: 'POST',
                                headers: {'Content-Type': 'application/json'},
                                body: JSON.stringify({email: email})
                            })
                            .then(response => response.json())
                            .then(data => {
                                alert(data.message || 'Retry initiated');
                                location.reload();
                            })
                            .catch(error => {
                                alert('Error: ' + error);
                            });
                        }
                    }
                    
                    // Auto-refresh every 30 seconds
                    setTimeout(() => location.reload(), 30000);
                </script>
            </body>
            </html>
            '''
            
            return dashboard_html
            
        except Exception as e:
            logger.error(f"Error loading onboarding dashboard: {e}")
            return f"Error loading dashboard: {e}", 500
    
    @app.route('/api/onboarding/test', methods=['POST'])
    def test_onboarding():
        """Test onboarding flow with fake data"""
        try:
            data = request.get_json()
            email = data.get('email', 'test@example.com')
            plan_type = data.get('plan_type', 'basic')
            
            # Create test onboarding event
            test_event = OnboardingEvent(
                event_id=str(uuid.uuid4()),
                customer_email=email,
                plan_type=plan_type,
                payment_amount=29.99,
                paypal_transaction_id=f"test_{int(time.time())}",
                timestamp=datetime.now()
            )
            
            # Start test onboarding
            threading.Thread(
                target=onboarding_manager._execute_onboarding_flow, 
                args=(test_event,),
                daemon=True
            ).start()
            
            return jsonify({
                'success': True, 
                'message': 'Test onboarding started',
                'event_id': test_event.event_id
            })
            
        except Exception as e:
            logger.error(f"Error testing onboarding: {e}")
            return jsonify({'error': 'Test failed'}), 500
    
    return onboarding_manager

# Customer Status Monitoring
class CustomerStatusMonitor:
    """Monitor customer status and health"""
    
    def __init__(self, customer_manager, whitelist_manager):
        self.customer_manager = customer_manager
        self.whitelist_manager = whitelist_manager
        
    def check_customer_health(self) -> Dict:
        """Check health status of all customers"""
        conn = sqlite3.connect('customer_resources.db')
        cursor = conn.cursor()
        
        # Get all active customers
        cursor.execute('''
            SELECT customer_id, email, plan_type, created_at 
            FROM customers 
            WHERE status = 'active'
        ''')
        
        customers = cursor.fetchall()
        health_report = {
            'total_customers': len(customers),
            'healthy_customers': 0,
            'warning_customers': 0,
            'critical_customers': 0,
            'customer_details': []
        }
        
        for customer_id, email, plan_type, created_at in customers:
            customer_health = self._check_individual_customer_health(customer_id)
            health_report['customer_details'].append({
                'customer_id': customer_id,
                'email': email,
                'plan_type': plan_type,
                'health_status': customer_health['status'],
                'issues': customer_health['issues']
            })
            
            if customer_health['status'] == 'healthy':
                health_report['healthy_customers'] += 1
            elif customer_health['status'] == 'warning':
                health_report['warning_customers'] += 1
            else:
                health_report['critical_customers'] += 1
        
        conn.close()
        return health_report
    
    def _check_individual_customer_health(self, customer_id: str) -> Dict:
        """Check health of individual customer"""
        issues = []
        status = 'healthy'
        
        try:
            # Check if customer has allocated resources
            resources = self.customer_manager.get_customer_resources(customer_id)
            if not resources:
                issues.append("No resources allocated")
                status = 'critical'
            
            # Check if IPs are whitelisted
            for resource in resources:
                is_allowed, _ = self.whitelist_manager.is_ip_allowed(resource.ip_address)
                if not is_allowed:
                    issues.append(f"IP {resource.ip_address} not whitelisted")
                    status = 'critical'
            
            # Check recent usage
            conn = sqlite3.connect('customer_resources.db')
            cursor = conn.cursor()
            
            # Check for recent activity (last 24 hours)
            cursor.execute('''
                SELECT COUNT(*) FROM usage_logs 
                WHERE customer_id = ? AND timestamp > ?
            ''', (customer_id, (datetime.now() - timedelta(hours=24)).isoformat()))
            
            recent_requests = cursor.fetchone()[0]
            
            # Check for high error rate
            cursor.execute('''
                SELECT 
                    COUNT(*) as total,
                    SUM(CASE WHEN success = 0 THEN 1 ELSE 0 END) as errors
                FROM usage_logs 
                WHERE customer_id = ? AND timestamp > ?
            ''', (customer_id, (datetime.now() - timedelta(hours=24)).isoformat()))
            
            usage_stats = cursor.fetchone()
            total_requests, error_count = usage_stats or (0, 0)
            
            if total_requests > 0:
                error_rate = error_count / total_requests
                if error_rate > 0.1:  # 10% error rate
                    issues.append(f"High error rate: {error_rate:.1%}")
                    status = 'warning' if status == 'healthy' else status
            
            conn.close()
            
        except Exception as e:
            issues.append(f"Health check error: {str(e)}")
            status = 'critical'
        
        return {
            'status': status,
            'issues': issues
        }

# Complete integration function
def setup_complete_onboarding_system(app, customer_manager, whitelist_manager, paypal_manager):
    """Setup complete automated onboarding system"""
    
    # Initialize onboarding manager
    onboarding_manager = integrate_automated_onboarding(
        app, customer_manager, whitelist_manager, paypal_manager
    )
    
    # Initialize monitoring
    status_monitor = CustomerStatusMonitor(customer_manager, whitelist_manager)
    
    @app.route('/api/system/health')
    def system_health_check():
        """System-wide health check"""
        try:
            health_report = status_monitor.check_customer_health()
            return jsonify(health_report)
        except Exception as e:
            logger.error(f"Health check failed: {e}")
            return jsonify({'error': 'Health check failed'}), 500
    
    # Background health monitoring
    def background_health_monitor():
        """Background task to monitor customer health"""
        while True:
            try:
                health_report = status_monitor.check_customer_health()
                
                # Alert on critical customers
                if health_report['critical_customers'] > 0:
                    logger.warning(f"Critical customers detected: {health_report['critical_customers']}")
                    # Send admin alert email here
                
                # Log health summary
                logger.info(
                    f"Customer Health: {health_report['healthy_customers']} healthy, "
                    f"{health_report['warning_customers']} warning, "
                    f"{health_report['critical_customers']} critical"
                )
                
                time.sleep(300)  # Check every 5 minutes
                
            except Exception as e:
                logger.error(f"Background health monitor error: {e}")
                time.sleep(300)
    
    # Start background monitoring
    health_thread = threading.Thread(target=background_health_monitor, daemon=True)
    health_thread.start()
    
    logger.info("🚀 Complete automated onboarding system initialized!")
    
    return {
        'onboarding_manager': onboarding_manager,
        'status_monitor': status_monitor
    }