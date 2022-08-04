DEVICE TREES FOR STREAM183x
===========================

axg_sue_s183x_internal.dtsi
-----------------------------

DT for components located on Stream183x module itself.


axg_sue_s183x_external_default.dtsi
-------------------------------------

DT for default configuration of Stream183x external IOs. This DT is excluded
from the StreamKit1832 L0 carrier board configuration. Some equivalent
components are defined in the carrier board DT.


axg_sue_s183x_external_factory.dtsi
-------------------------------------

DT for factory configuration of Stream183x external IOs.

This file is empty for now, as all Stream183x outputs are by default GPIOs
which is exactly what we need.



DEVICE TREES FOR CARRIER BOARDS
===============================

axg_sue_prime.dtsi
------------------

DT for components located on StreamKit prime carrier board or daughter
boards which are not detectable via ADC.


axg_sue_factory.dtsi
--------------------
DT for components located on factory tester carrier board.

This file is empty for now, as there is nothing on the factory module, which
would require device tree entry.

axg_sue_streamkit1832.dtsi
-----------------------------
DT for components located on StreamKit1832 carrier board without any daughter
board.

Everything that is normally defined in external default is defined here. The
default DT provide conflicts with daughter boards DT.

