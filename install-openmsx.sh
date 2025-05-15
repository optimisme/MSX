brew install openmsx
mkdir -p ~/.openMSX/share/machines/
cp ./Philips_VG8020/*.xml ~/.openMSX/share/machines/
mkdir -p ~/.openMSX/share/systemroms/
cp ./Philips_VG8020/*.rom ~/.openMSX/share/systemroms/
openmsx -machine Philips_VG8020