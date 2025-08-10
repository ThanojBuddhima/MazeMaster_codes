import serial
import pygame
import time
import tkinter as tk
from tkinter import ttk, filedialog, messagebox
import threading
import os
import glob

class ArduinoAudioInterface:
    def __init__(self):
        self.root = tk.Tk()
        self.root.title("Maze Master Player Interface")
        self.root.geometry("600x500")
        self.root.configure(bg='#2c3e50')
        
        # Serial connection
        self.serial_connection = None
        self.is_connected = False
        self.is_listening = False
        
        # Audio files
        self.audio_files = {
            'TRACK_1': None,
            'TRACK_2': None,
            'TRACK_3': None,
            'WIN_RED': None,
            'WIN_BLUE': None
        }
        
        # Initialize pygame mixer
        pygame.mixer.init(frequency=22050, size=-16, channels=2, buffer=512)
        
        self.setup_ui()
        
    def setup_ui(self):
        # Title
        title_label = tk.Label(self.root, text="Maze Master Player", 
                              font=("Arial", 20, "bold"), 
                              bg='#2c3e50', fg='#ecf0f1')
        title_label.pack(pady=10)
        
        # Connection Frame
        conn_frame = tk.Frame(self.root, bg='#2c3e50')
        conn_frame.pack(pady=10, padx=20, fill='x')
        
        tk.Label(conn_frame, text="Serial Port:", 
                bg='#2c3e50', fg='#ecf0f1', font=("Arial", 10)).pack(side='left')
        
        self.port_var = tk.StringVar()
        self.port_combo = ttk.Combobox(conn_frame, textvariable=self.port_var, 
                                      state='readonly', width=15)
        self.port_combo.pack(side='left', padx=(5, 10))
        
        self.refresh_ports_btn = tk.Button(conn_frame, text="Refresh", 
                                          command=self.refresh_ports,
                                          bg='#3498db', fg='white', font=("Arial", 9))
        self.refresh_ports_btn.pack(side='left', padx=5)
        
        self.connect_btn = tk.Button(conn_frame, text="Connect", 
                                    command=self.toggle_connection,
                                    bg='#27ae60', fg='white', font=("Arial", 10, "bold"))
        self.connect_btn.pack(side='left', padx=10)
        
        self.status_label = tk.Label(conn_frame, text="Disconnected", 
                                    bg='#2c3e50', fg='#e74c3c', font=("Arial", 10))
        self.status_label.pack(side='left', padx=10)
        
        # Audio Files Frame
        audio_frame = tk.LabelFrame(self.root, text="Audio Files Configuration", 
                                   bg='#34495e', fg='#ecf0f1', font=("Arial", 12, "bold"))
        audio_frame.pack(pady=20, padx=20, fill='both', expand=True)
        
        # Create audio file selection rows
        self.audio_labels = {}
        self.audio_buttons = {}
        
        for i, (key, description) in enumerate([
            ('TRACK_1', 'Sequence 1 Audio'),
            ('TRACK_2', 'Sequence 2 Audio'),
            ('TRACK_3', 'Sequence 3 Audio'),
            ('WIN_RED', 'Red Team Win Audio'),
            ('WIN_BLUE', 'Blue Team Win Audio')
        ]):
            row_frame = tk.Frame(audio_frame, bg='#34495e')
            row_frame.pack(fill='x', padx=10, pady=5)
            
            tk.Label(row_frame, text=f"{description}:", 
                    bg='#34495e', fg='#ecf0f1', font=("Arial", 10),
                    width=20, anchor='w').pack(side='left')
            
            self.audio_labels[key] = tk.Label(row_frame, text="No file selected", 
                                            bg='#34495e', fg='#95a5a6', font=("Arial", 9),
                                            anchor='w')
            self.audio_labels[key].pack(side='left', fill='x', expand=True, padx=10)
            
            self.audio_buttons[key] = tk.Button(row_frame, text="Browse", 
                                              command=lambda k=key: self.browse_audio_file(k),
                                              bg='#3498db', fg='white', font=("Arial", 9))
            self.audio_buttons[key].pack(side='right')
        
        # Control Frame
        control_frame = tk.Frame(self.root, bg='#2c3e50')
        control_frame.pack(pady=10, padx=20, fill='x')
        
        self.test_btn = tk.Button(control_frame, text="Test All Sounds", 
                                 command=self.test_all_sounds,
                                 bg='#f39c12', fg='white', font=("Arial", 10, "bold"))
        self.test_btn.pack(side='left', padx=5)
        
        self.stop_btn = tk.Button(control_frame, text="Stop Audio", 
                                 command=self.stop_audio,
                                 bg='#e74c3c', fg='white', font=("Arial", 10, "bold"))
        self.stop_btn.pack(side='left', padx=5)
        
        # Log Frame
        log_frame = tk.LabelFrame(self.root, text="Activity Log", 
                                 bg='#34495e', fg='#ecf0f1', font=("Arial", 12, "bold"))
        log_frame.pack(pady=10, padx=20, fill='both', expand=True)
        
        self.log_text = tk.Text(log_frame, height=8, bg='#2c3e50', fg='#ecf0f1', 
                               font=("Courier", 9), wrap='word')
        scrollbar = tk.Scrollbar(log_frame, command=self.log_text.yview)
        self.log_text.configure(yscrollcommand=scrollbar.set)
        
        self.log_text.pack(side='left', fill='both', expand=True, padx=5, pady=5)
        scrollbar.pack(side='right', fill='y', pady=5)
        
        # Initialize
        self.refresh_ports()
        self.log("Arduino Audio Interface initialized")
        
    def refresh_ports(self):
        """Refresh available serial ports"""
        ports = []
        
        # Windows
        for i in range(256):
            try:
                s = serial.Serial(f'COM{i}')
                s.close()
                ports.append(f'COM{i}')
            except (OSError, serial.SerialException):
                pass
        
        # Linux/Mac
        for port in glob.glob('/dev/tty[A-Za-z]*'):
            ports.append(port)
        
        self.port_combo['values'] = ports
        if ports:
            self.port_combo.set(ports[0])
            
    def toggle_connection(self):
        """Toggle serial connection"""
        if self.is_connected:
            self.disconnect()
        else:
            self.connect()
            
    def connect(self):
        """Connect to Arduino"""
        try:
            if not self.port_var.get():
                messagebox.showerror("Error", "Please select a serial port")
                return
                
            self.serial_connection = serial.Serial(
                port=self.port_var.get(),
                baudrate=9600,
                timeout=1
            )
            
            self.is_connected = True
            self.status_label.config(text="Connected", fg='#27ae60')
            self.connect_btn.config(text="Disconnect", bg='#e74c3c')
            
            # Start listening thread
            self.is_listening = True
            self.listen_thread = threading.Thread(target=self.listen_to_arduino, daemon=True)
            self.listen_thread.start()
            
            self.log(f"Connected to {self.port_var.get()}")
            
        except Exception as e:
            messagebox.showerror("Connection Error", f"Failed to connect: {str(e)}")
            self.log(f"Connection failed: {str(e)}")
            
    def disconnect(self):
        """Disconnect from Arduino"""
        self.is_listening = False
        if self.serial_connection:
            self.serial_connection.close()
            self.serial_connection = None
            
        self.is_connected = False
        self.status_label.config(text="Disconnected", fg='#e74c3c')
        self.connect_btn.config(text="Connect", bg='#27ae60')
        self.log("Disconnected from Arduino")
        
    def listen_to_arduino(self):
        """Listen for audio commands from Arduino"""
        while self.is_listening and self.is_connected:
            try:
                if self.serial_connection.in_waiting > 0:
                    line = self.serial_connection.readline().decode('utf-8').strip()
                    
                    # Log all Arduino output
                    if line and not line.startswith("AUDIO:"):
                        self.log(f"Arduino: {line}")
                    
                    # Check for audio commands
                    if line.startswith("AUDIO:"):
                        audio_command = line.replace("AUDIO:", "")
                        self.play_audio(audio_command)
                        
                time.sleep(0.1)  # Small delay to prevent excessive CPU usage
                
            except Exception as e:
                if self.is_listening:  # Only log if we're still supposed to be listening
                    self.log(f"Error reading from Arduino: {str(e)}")
                break
                
    def browse_audio_file(self, audio_key):
        """Browse for audio file"""
        file_path = filedialog.askopenfilename(
            title=f"Select audio file for {audio_key}",
            filetypes=[
                ("Audio Files", "*.mp3 *.wav *.ogg *.flac"),
                ("MP3 Files", "*.mp3"),
                ("WAV Files", "*.wav"),
                ("OGG Files", "*.ogg"),
                ("All Files", "*.*")
            ]
        )
        
        if file_path:
            self.audio_files[audio_key] = file_path
            filename = os.path.basename(file_path)
            self.audio_labels[audio_key].config(text=filename, fg='#2ecc71')
            self.log(f"Set {audio_key}: {filename}")
            
    def play_audio(self, audio_command):
        """Play audio based on command"""
        if audio_command in self.audio_files:
            audio_file = self.audio_files[audio_command]
            if audio_file and os.path.exists(audio_file):
                try:
                    pygame.mixer.music.load(audio_file)
                    pygame.mixer.music.play()
                    self.log(f"Playing: {audio_command} - {os.path.basename(audio_file)}")
                except Exception as e:
                    self.log(f"Error playing {audio_command}: {str(e)}")
            else:
                self.log(f"Audio file not set or not found for: {audio_command}")
        else:
            self.log(f"Unknown audio command: {audio_command}")
            
    def stop_audio(self):
        """Stop currently playing audio"""
        pygame.mixer.music.stop()
        self.log("Audio stopped")
        
    def test_all_sounds(self):
        """Test all configured audio files"""
        for key, file_path in self.audio_files.items():
            if file_path and os.path.exists(file_path):
                self.log(f"Testing {key}...")
                try:
                    pygame.mixer.music.load(file_path)
                    pygame.mixer.music.play()
                    time.sleep(2)  # Play for 2 seconds
                    pygame.mixer.music.stop()
                except Exception as e:
                    self.log(f"Error testing {key}: {str(e)}")
                time.sleep(1)  # Pause between tests
            else:
                self.log(f"No file set for {key}")
                
    def log(self, message):
        """Add message to log"""
        timestamp = time.strftime("%H:%M:%S")
        self.log_text.insert('end', f"[{timestamp}] {message}\n")
        self.log_text.see('end')
        self.root.update_idletasks()
        
    def run(self):
        """Start the application"""
        self.root.protocol("WM_DELETE_WINDOW", self.on_closing)
        self.root.mainloop()
        
    def on_closing(self):
        """Handle application closing"""
        if self.is_connected:
            self.disconnect()
        pygame.mixer.quit()
        self.root.destroy()

if __name__ == "__main__":
    app = ArduinoAudioInterface()
    app.run()
