Summary: CannonSmash is a 3D tabletennis game.
Name: csmash
%define version 0.4.6
Version: %{version}
Release: 1
Group: Games
Copyright: GPL
Source: csmash-%{version}.tar.gz
URL: http://cannonsmash.sourceforge.net/
Packager: Marc Lavallée <odradek@videotron.ca>
Requires: Mesa >= 3.1

%description
CannonSmash is a 3D tabletennis game. 
The goal of this project is to represent various strategy 
of tabletennis on computer game.

%prep
%setup

%build
./configure --prefix=/usr
make

%install
make install

%files
/usr/bin/csmash
/usr/share/csmash
%doc AUTHORS COPYING ChangeLog NEWS README README.nt

