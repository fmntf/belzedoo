from termcolor import colored
import serial

class BelzeTest:

	port = None

	def openport(self):
		self.port = serial.Serial('/dev/ttyMCC', 115200, timeout=1);
		print(self.port.readline());

	def write(self, message, read=True):
		print colored(message, 'green');
		self.port.write(message + chr(13));
		self.port.flushOutput();
		if read:
			print colored(self.port.readline(), 'yellow');
	
	def read(self):
		print colored(self.port.readline(), 'yellow');
	
	def closeport(self):
		self.port.readline();
		self.port.close();

