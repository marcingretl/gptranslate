import matplotlib.pyplot as plt
from matplotlib.ticker import MultipleLocator, FormatStrFormatter, NullFormatter, AutoMinorLocator
from matplotlib.colors import LinearSegmentedColormap, Normalize
fig, axes = plt.subplots(1, 1, squeeze=False, figsize=(6.4,4.8), dpi=100)
axes = axes.flatten()
ax = axes[0]
target = ax
x = [-0.11992409549999999,-0.1153962722,-0.11086844899999999,-0.1063406258,-0.1018128026,-0.097284979339999994,-0.092757156110000002,-0.088229332889999998,-0.083701509660000006,-0.079173686430000001,-0.074645863209999996,-0.070118039980000005,-0.06559021676,-0.061062393530000002,-0.056534570300000003,-0.052006747079999999,-0.04747892385,-0.042951100620000002,-0.038423277399999997,-0.033895454169999999,-0.02936763094,-0.024839807719999999,-0.020311984490000001,-0.015784161259999999,-0.011256338039999999,-0.0067285148110000003,-0.0022006915840000001,0.0023271316420000001,0.0068549548690000003]
y = [0.001222493888,0,0.001222493888,0,0,0,0.018337408310000001,0.02567237164,0.05501222494,0.042787286059999999,0.058679706599999999,0.046454767729999999,0.057457212709999997,0.07457212714,0.061124694379999997,0.059902200490000002,0.091687041560000002,0.0293398533,0.10146699269999999,0.040342298290000002,0.066014669930000006,0.0097799510999999999,0.031784841080000001,0.0195599022,0.030562347189999998,0.028117359410000001,0.0036674816630000002,0.023227383859999999,0.022004889980000001]
kw = {'label': '_nolegend_'}
kw['color'] = '#5F6B84'
kw['linewidth'] = 1.5
_box_steps = [abs(x[i+1] - x[i]) for i in range(len(x)-1) if x[i+1] != x[i]]
_box_spacing = min(_box_steps) if _box_steps else 1.0
_box_width = 0.8 * _box_spacing
_box_centers, _box_heights, _box_widths = [], [], []
_box_i = 0
while _box_i < len(x):
    _box_j = _box_i + 1
    while _box_j < len(x) and y[_box_j] == y[_box_i] and abs((x[_box_j] - x[_box_j-1]) - _box_spacing) <= max(1e-3 * _box_spacing, 1e-12):
        _box_j += 1
    if _box_j - _box_i >= 3:
        if y[_box_i] != 0:
            _box_centers.append((x[_box_i] + x[_box_j-1]) / 2)
            _box_heights.append(y[_box_i])
            _box_widths.append(x[_box_j-1] - x[_box_i] + _box_spacing)
    else:
        for _box_k in range(_box_i, _box_j):
            if y[_box_k] != 0:
                _box_centers.append(x[_box_k])
                _box_heights.append(y[_box_k])
                _box_widths.append(_box_width)
    _box_i = _box_j
target.bar(_box_centers, _box_heights, width=_box_widths, alpha=0.6, zorder=1, **kw)
for side, bit in [('bottom',1),('left',2),('top',4),('right',8)]:
    ax.spines[side].set_visible(bool(15 & bit))
ax.set_title('')
ax.set_xlabel('HE')
ax.set_ylabel('Relative frequency')
ax.set_xlim(left=-0.124452)
ax.set_xlim(right=0.0113828)
ax.set_ylim(bottom=0)
ax.set_ylim(top=0.111614)
for ax in axes[1:]: ax.set_visible(False)
fig.tight_layout()
plt.show()
