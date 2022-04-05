import matplotlib.pyplot as plt
from aihwkit.utils.visualization import plot_device_compact
from aihwkit.simulator.configs.devices import SelfDefineDevice

# define up/down pulse vs weight relationship
# n number of points in array
n_points = 5
up_pulse = [0.001, 0.002, 0.003, 0.004, 0.005]
down_pulse = [0.005, 0.004, 0.003, 0.002, 0.001]

plt.ion()
plot_device_compact(
    SelfDefineDevice(w_min=-1, w_max=1, dw_min=0.01, pow_gamma=1.1, pow_gamma_dtod=0.5,
                  pow_up_down=0.2, w_min_dtod=0.0, w_max_dtod=0.0, n_points = n_points,
                                                                   up_pulse = up_pulse,
                                                                   down_pulse = down_pulse), n_steps=1000)
# plt.show()
plt.savefig('my_figure.png')