#! /usr/bin/env python3

# import requests
import re

import matplotlib.pyplot as plt
import numpy as np
import scipy as sp
import scipy.signal

SAMPLING_RATE = 1000 / 25.

# URL = "http://192.168.1.171"
LINE_RE = re.compile(r"([0-9]+),([0-9]+)")


def parse_line(l):
    m = LINE_RE.fullmatch(l)
    # print(l, m)
    return int(m.group(1)), int(m.group(2))


def get_data():
    # r = requests.get(URL, allow_redirects=True)
    # t = r.text
    with open("/media/router/storage/out.csv", "rt") as f:
        t = f.read()
        return [parse_line(l) for l in t.splitlines(keepends=False)[-1024*10:]]


filter = sp.signal.firwin(800, 1/3, fs=SAMPLING_RATE)


# print(filter)

# the function below is for updating both x and y values (great for updating dates on the x-axis)
def live_plotter_xy(x_vec, y1_data, line1, identifier='', pause_time=0.01, xmax=None):
    if line1 == []:
        plt.ion()
        fig = plt.figure(figsize=(13, 6))
        ax = fig.add_subplot(111)
        line1, = ax.plot(x_vec, y1_data, 'r-o', alpha=0.8)
        plt.ylabel('Y Label')
        plt.title('Title: {}'.format(identifier))
        plt.show()

    line1.set_data(x_vec, y1_data)
    plt.xlim(np.min(x_vec), xmax or np.max(x_vec))
    if np.min(y1_data) <= line1.axes.get_ylim()[0] or np.max(y1_data) >= line1.axes.get_ylim()[1]:
        plt.ylim([np.min(y1_data) - np.std(y1_data), np.max(y1_data) + np.std(y1_data)])

    plt.pause(pause_time)

    return line1


def autocorrelate(x):
    result = np.correlate(x, x, mode='full')
    return result[result.size // 2:]


data = get_data()

d = np.array(data)
dx = (d[:, 0] - d[0, 0]) / 1000.0
dy = scipy.signal.lfilter(filter, 1.0, d[:, 1])
# df, dp = sp.signal.periodogram(d[:, 1], fs=SAMPLING_RATE, nfft=1000 * 10 + 1)
# dc = autocorrelate(dy)
# print(dy.shape, dc.shape)
history = live_plotter_xy(dx, dy, [], pause_time=60)
# periodogram = live_plotter_xy(df, dp, periodogram, pause_time=0.5, xmax=1)
# autocorrogram = live_plotter_xy(np.arange(0, dc.shape[0]) / SAMPLING_RATE, dc / dc.max(), autocorrogram, pause_time=0.5, xmax=60)


if False:
    size = 100
    history = []
    periodogram = []
    autocorrogram = []
    data = []
    while True:
        data.extend(get_data())
        d = np.array(data)
        dx = (d[:, 0]- d[0, 0]) / 1000.0
        dy = scipy.signal.lfilter(filter, 1.0, d[:, 1])
        # df, dp = sp.signal.periodogram(d[:, 1], fs=SAMPLING_RATE, nfft=1000 * 10 + 1)
        # dc = autocorrelate(dy)
        # print(dy.shape, dc.shape)
        history = live_plotter_xy(dx, dy, history, pause_time=0.5)
        # periodogram = live_plotter_xy(df, dp, periodogram, pause_time=0.5, xmax=1)
        # autocorrogram = live_plotter_xy(np.arange(0, dc.shape[0]) / SAMPLING_RATE, dc / dc.max(), autocorrogram, pause_time=0.5, xmax=60)
