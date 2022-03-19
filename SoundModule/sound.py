import librosa
import soundfile
from aip import AipSpeech
import pyaudio
import wave
import re
import os
import time

APP_ID = '25770534'
API_KEY = 'E72uiFYP1fOrzZDWRXjkDqoi'
SECRET_KEY = 'StM3I568txNCsxpBgDb760W2ybfF38mj'

client = AipSpeech(APP_ID, API_KEY, SECRET_KEY)


# 读取文件
def get_file_content(filePath):
    with open(filePath, 'rb') as fp:
        return fp.read()


def start_audio(time=5, save_file="test1.wav"):
    CHUNK = 1024
    FORMAT = pyaudio.paInt16
    CHANNELS = 1
    RATE = 48000
    RECORD_SECONDS = time  # 需要录制的时间
    WAVE_OUTPUT_FILENAME = save_file  # 保存的文件名

    p = pyaudio.PyAudio()  # 初始化
    print("ON")

    stream = p.open(format=FORMAT,
                    channels=CHANNELS,
                    rate=RATE,
                    input=True,
                    frames_per_buffer=CHUNK)  # 创建录音文件
    frames = []

    for i in range(0, int(RATE / CHUNK * RECORD_SECONDS)):
        data = stream.read(CHUNK, exception_on_overflow=False)
        frames.append(data)  # 开始录音
    stream.stop_stream()
    stream.close()
    p.terminate()
    print("OFF")

    wf = wave.open(WAVE_OUTPUT_FILENAME, 'wb')  # 保存
    wf.setnchannels(CHANNELS)
    wf.setsampwidth(p.get_sample_size(FORMAT))
    wf.setframerate(RATE)
    wf.writeframes(b''.join(frames))
    wf.close()
    resample_rate('test1.wav')


def resample_rate(path, new_sample_rate=16000):
    signal, sr = librosa.load(path, sr=None)
    file_name = 'new.wav'
    new_signal = librosa.resample(signal, sr, new_sample_rate)
    # librosa.write_wav(file_name, new_signal, new_sample_rate)
    soundfile.write(file_name, new_signal, new_sample_rate)


def run_video():
    start_audio()
    # 识别本地文件
    text = client.asr(get_file_content('new.wav'), 'wav', 16000, {
        'dev_pid': 1537,
    })
    word = text.get('result')[0][:-1]
    print(word)
    return word


def main():
    # start_audio(time=5, save_file="test1.wav")
    run_video()
    # resample_rate('test1.wav')

if __name__ == '__main__':
    main()
