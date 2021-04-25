# Get Updates and Install Upgrades
sudo apt-get update
sudo apt-get upgrade

# INSTALL ECLIPSE, GIT, MELD, EIGEN3, AND ARVDUDE FFMPEG VOKOSCREEN (the last two are for screen capture)
sudo apt-get install -y build-essential libeigen3-dev git meld gcc-avr avr-libc avrdude ffmpeg vokoscreen
sudo dpkg --configure -a

# Install Python and libraries
sudo apt-get install -y python3 libatlas-base-dev python3-numpy python3-pandas python3-serial python3-matplotlib 
echo "export Path=/home/pi/.local/bin:$Path" >> ~/.bashrc
. ~/.bashrc

# CLONE CLASS REPOS AND SETUP FILE SYSTEM
cd ~/Desktop
git clone https://github.com/M3R-CSM/MEGN540.git
cd MEGN540
git submodule update --init --recursive
cd ..

# Remove Mode Manager Which Causes Issues with programming
sudo apt-get purge modemmanager
sudo apt-get autoremove

# SETUP UDEV RULES FOR ZUMO
sudo sh -c 'echo "ACTION==\"add\", SUBSYSTEM==\"tty\", ATTRS{idVendor}==\"1ffb\", ATTRS{idProduct}==\"2300\", SYMLINK+=\"ttyZumoCar\"
ACTION==\"add\", SUBSYSTEM==\"tty\", ATTRS{idVendor}==\"1ffb\", ATTRS{idProduct}==\"0101\", SYMLINK+=\"ttyZumoCarAVR\"
SUBSYSTEM==\"usb\", ATTRS{idVendor}==\"1ffb\", GROUP=\"dialout\", MODE=\"0666\" " > /etc/udev/rules.d/99-zumo.rules'

sudo udevadm trigger
