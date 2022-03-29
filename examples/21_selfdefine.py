import matplotlib.pyplot as plt
from aihwkit.utils.visualization import plot_device_compact
from aihwkit.simulator.configs.devices import SelfDefineDevice

# define up/down pulse vs weight relationship
# n number of points
up_pulse = [0.001, 0.002, 0.003, 0.004, 0.005]
# up_weight = [1.0, 0.5, 0.0, -0.5, -1.0]

down_pulse = [0.001, 0.002, 0.003, 0.004, 0.005]
# down_weight = [-1.0, -0.5, 0.0, 0.5, 1.0]
n_points = 5

# n points within min/max range
# automatically determine points
# dw_min, dw_max range of inputs

# plt.ion()
print("python test")
plt.ioff()
plot_device_compact(
    SelfDefineDevice(w_min=-1, w_max=1, dw_min=1.0, w_min_dtod=0.0, w_max_dtod=0.0, dw_min_std=0.0, dw_min_dtod=0.0, up_down_dtod=0.0,
                                                                   sd_up_pulse=up_pulse, 
                                                                #    sd_up_weight=up_weight, 
                                                                   sd_down_pulse=down_pulse, 
                                                                #    sd_down_weight=down_weight,
                                                                   sd_n_points=n_points), n_steps=10)
'''
no dtod params

dw_min=1.0,
dw_min_dtod=0.0,
dw_min_std=0.0,
up_down=0.0,
up_down_dtod=0.0,
w_max=1,
w_max_dtod=0.0,
w_min=-1,
w_min_dtod=0.0
'''
# plt.show()
plt.savefig('my_figure.png')