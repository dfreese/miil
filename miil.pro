TEMPLATE = subdirs
SUBDIRS += \
    miil_core \
    miil_ftdi \
    miil_pcap \
    miil_gui \
    miil_config \
    miil_process \
    miil_all

miil_ftdi.depends = miil_core
miil_pcap.depends = miil_core
miil_gui.depends = miil_core
miil_config.depends = miil_core
miil_process.depends = miil_core miil_config
miil_all.depends = miil_core miil_ftdi miil_pcap miil_gui miil_config
