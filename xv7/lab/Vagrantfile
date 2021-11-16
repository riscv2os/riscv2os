Vagrant.configure(2) do |config|

  config.vm.box = 'generic/arch'

  # synced folder
  config.vm.synced_folder '.', '/xv6'

  # disable default synced folder
  config.vm.synced_folder '.', '/vagrant', disabled: true

  # install packages
  config.vm.provision 'shell', inline: <<-EOS
    pacman -Syq --noconfirm --needed \
      riscv64-linux-gnu-binutils \
      riscv64-linux-gnu-gcc \
      riscv64-linux-gnu-gdb \
      qemu-headless-arch-extra \
      base-devel \
      git
  EOS

end
