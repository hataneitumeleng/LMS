import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.patches import FancyBboxPatch, Circle

fig, ax = plt.subplots(figsize=(10, 9.4), dpi=220)
ax.set_xlim(0, 100); ax.set_ylim(0, 94); ax.axis('off')
FS = 8.6
LW = 1.0

def state(cx, cy, w, h, name, entry=None):
    ax.add_patch(FancyBboxPatch((cx-w/2, cy-h/2), w, h, boxstyle='round,pad=0,rounding_size=1.6',
                                fc='#F1F1F1', ec='black', lw=LW))
    if entry:
        ax.text(cx, cy+h/4, name, ha='center', va='center', fontsize=FS+0.6)
        ax.plot([cx-w/2, cx+w/2], [cy, cy], color='black', lw=0.8)
        ax.text(cx, cy-h/4, entry, ha='center', va='center', fontsize=FS-0.4)
    else:
        ax.text(cx, cy, name, ha='center', va='center', fontsize=FS+0.6)
    return dict(l=cx-w/2, r=cx+w/2, t=cy+h/2, b=cy-h/2, cx=cx, cy=cy)

def arrow(pts, label=None, lx=None, ly=None, ha='left'):
    xs, ys = zip(*pts)
    ax.plot(xs[:-1] + (xs[-1],), ys, color='black', lw=LW)
    ax.annotate('', xy=pts[-1], xytext=pts[-2],
                arrowprops=dict(arrowstyle='-|>', color='black', lw=LW, mutation_scale=11, shrinkA=0, shrinkB=0))
    if label:
        ax.text(lx, ly, label, fontsize=FS, ha=ha, va='center',
                bbox=None if ha == 'center' else dict(fc='white', ec='none', pad=0.6))

# states
S = state(22, 68, 18, 6, 'Submitted')
L = state(70, 68, 40, 9, 'Late', 'entry / daysLate = started 24 h periods (BR-02)')
M = state(46, 46, 46, 9, 'Marked', 'entry / finalMark = policy.finalMark(raw, max, daysLate)')
Mo = state(46, 28, 18, 6, 'Moderated')
R = state(46, 12, 42, 9, 'Released', 'entry / marks and feedback visible (BR-07)')

# initial
ax.add_patch(Circle((46, 89), 1.6, color='black'))
arrow([(46, 87.4), (46, 82), (22, 82), (S['cx'], S['t'])], 'submit [now <= effective deadline]', 21, 77.5, ha='right')
arrow([(46, 82), (70, 82), (70, L['t'])], 'submit [now > effective deadline]', 54, 84.3)

# Late -> Submitted (extension covers it)
arrow([(L['l'], 69.5), (S['r'], 69.5)], 'extension granted\n[submittedAt <= new deadline]\n/ daysLate = 0', 40.5, 74.8, ha='center')
# Late self loop (right side, rectangular)
arrow([(L['r'], 70.5), (94, 70.5), (94, 65.5), (L['r'], 65.5)], None)
ax.text(95.5, 68, 'extension granted\n[submittedAt > new\ndeadline] / recalc\ndaysLate (BR-04)', fontsize=FS-0.4, ha='left', va='center')
# Submitted self loop (left)
arrow([(S['l'], 69.5), (8, 69.5), (8, 66.5), (S['l'], 66.5)], None)
ax.text(1, 61.5, 'resubmit [now <= effective\ndeadline] / versionNo++', fontsize=FS-0.4, ha='left', va='center')

# to Marked
arrow([(28, S['b']), (28, M['t'])], 'mark(raw, feedback)', 29, 58)
arrow([(64, L['b']), (64, M['t'])], 'mark(raw, feedback)', 65, 58)
# Marked self loop (right)
arrow([(M['r'], 48.5), (78, 48.5), (78, 43.5), (M['r'], 43.5)], None)
ax.text(79.5, 46, 'amend(raw, feedback, reason)\n/ audit (FR-18)', fontsize=FS-0.4, ha='left', va='center')
# Marked -> Moderated
arrow([(42, M['b']), (42, Mo['t'])], 'approve / record HoD\nand time', 41, 36.5, ha='right')
arrow([(50, M['b']), (50, Mo['t'])], 'adjust(final, reason)\n/ audit', 51, 36.5)
# Moderated -> Released
arrow([(46, Mo['b']), (46, R['t'])], 'release [all submissions of the\nassignment are Moderated]', 47, 21)
# Released self loop (right)
arrow([(R['r'], 14.5), (76, 14.5), (76, 9.5), (R['r'], 9.5)], None)
ax.text(77.5, 12, 'HoD change(final, reason)\n/ audit (BR-08)', fontsize=FS-0.4, ha='left', va='center')
plt.savefig('state.png', bbox_inches='tight', facecolor='white')
