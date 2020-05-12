#include "EyerWand.hpp"
#include <math.h>

namespace Eyer
{
    EyerVideoDecoderLine::EyerVideoDecoderLine(EyerString _resPath, double _initStart, EyerAVStreamType type)
    {
        initStart = _initStart;
        resPath = _resPath;
        reader = new EyerAVReader(resPath);
        int ret = reader->Open();
        if(ret){
            reader->Close();
            delete reader;
            reader = nullptr;
        }

        int streamCount = reader->GetStreamCount();
        for(int i=0;i<streamCount;i++){
            EyerAVStream stream;
            ret = reader->GetStream(stream, i);
            if(ret){
                continue;
            }

            if(stream.GetStreamType() == type){
                videoStreamIndex = i;
            }
        }

        EyerAVStream avStream;
        ret = reader->GetStream(avStream, videoStreamIndex);

        decoder = new EyerAVDecoder();
        decoder->Init(&avStream);

        reader->SeekFrame(-1, (int64_t)(initStart * 1000000));
    }

    EyerVideoDecoderLine::~EyerVideoDecoderLine()
    {
        if(reader != nullptr){
            reader->Close();
            delete reader;
            reader = nullptr;
        }

        if(decoder != nullptr){
            delete decoder;
            decoder = nullptr;
        }

        for(int i=0;i<frameList.getLength();i++){
            EyerAVFrame * f = nullptr;
            frameList.find(i, f);
            if(f != nullptr){
                delete f;
            }
        }
        frameList.clear();
    }

    int EyerVideoDecoderLine::GetCacheFrameCount()
    {
        return frameList.getLength();
    }

    int EyerVideoDecoderLine::GetFrame(EyerAVFrame & frame, double ts)
    {
        if(ts < GetStartTime()){
            // EyerLog("ts < initStart\n");
            // EyerLog("ts:%f, initStart:%f", ts, initStart);
            return -1;
        }

        EyerAVFrame * pFrame = nullptr;

        while(1){
            int ret = SelectFrameInList(pFrame, ts);
            if(ret){
                // 先判断是否已经读取到文件末尾
                if(fileEndFlag){
                    int length = frameList.getLength();
                    // 已经到了末尾，取最后一帧，退出
                    if(frameList.getLength() <= 0){
                        continue;
                        // break;
                    }
                    int lastInex = frameList.getLength() - 1;
                    frameList.find(lastInex, pFrame);

                    break;
                }
                ReadFrame();
                continue;
            }

            // 读取成功
            break;
        }

        if(pFrame == nullptr){
            return -1;
        }

        frame = *pFrame;

        Eyer::EyerAVRational streamTimebase;
        reader->GetStreamTimeBase(streamTimebase, videoStreamIndex);

        double t = frame.GetPTS() * 1.0 * streamTimebase.num / streamTimebase.den;

        // EyerLog("Target TS: %f, Frame TS: %f, D: %f\n", ts, t, t - ts);

        // EyerLog("List Size: %d\n", frameList.getLength());

        return 0;
    }

    double EyerVideoDecoderLine::GetStartTime()
    {
        if(frameList.getLength() <= 0){
            return initStart;
        }

        EyerAVFrame * f = nullptr;
        frameList.find(0, f);
        if(f == nullptr){
            return initStart;
        }

        Eyer::EyerAVRational streamTimebase;
        reader->GetStreamTimeBase(streamTimebase, videoStreamIndex);

        double t = f->GetPTS() * 1.0 * streamTimebase.num / streamTimebase.den;

        return t;
    }

    int EyerVideoDecoderLine::SelectFrameInList(EyerAVFrame * & frame, double ts)
    {
        EyerAVFrame * ff = nullptr;
        for(int i=0;i<frameList.getLength();i++){
            EyerAVFrame * f = nullptr;
            frameList.find(i, f);
            if(f != nullptr){
                Eyer::EyerAVRational streamTimebase;
                reader->GetStreamTimeBase(streamTimebase, videoStreamIndex);

                double t = f->GetPTS() * 1.0 * streamTimebase.num / streamTimebase.den;

                if(t > ts){
                    if(i <= 1){
                        ff = f;
                    }
                    else{
                        // 向前检索一帧进行比较
                        EyerAVFrame * lastF = nullptr;
                        frameList.find(i - 1, lastF);

                        double lastFT = lastF->GetPTS() * 1.0 * streamTimebase.num / streamTimebase.den;

                        if(abs(lastFT - ts) < abs(t - ts)){
                            ff = lastF;
                        }
                        else{
                            ff = f;
                        }
                    }

                    break;
                }
                if(t == ts){
                    ff = f;
                    break;
                }
            }
        }

        // 无论成功还是失败，都要丢帧
        while(frameList.getLength() > 5){
            EyerAVFrame * f = nullptr;
            frameList.find(0, f);
            if(f != nullptr){
                delete f;
            }
            frameList.deleteEle(0);
        }

        if(ff == nullptr){
            
            return -1;
        }

        frame = ff;

        return 0;
    }

    int EyerVideoDecoderLine::ReadFrame()
    {
        if(reader == nullptr){
            return -1;
        }
        EyerAVPacket pkt;
        int ret = reader->Read(&pkt);
        // EyerLog("Read Frame: %d\n", ret);
        if(pkt.GetStreamId() != videoStreamIndex){
            return -1;
        }
        if(ret){
            // 清空解码器
            decoder->SendPacket(nullptr);
            while(1){
                EyerAVFrame * frame = new EyerAVFrame();
                ret = decoder->RecvFrame(frame);
                if(ret){
                    delete frame;
                    break;
                }

                frameList.insertBack(frame);
            }

            fileEndFlag = 1;
        }
        else{
            decoder->SendPacket(&pkt);
            while(1){
                EyerAVFrame * frame = new EyerAVFrame();
                ret = decoder->RecvFrame(frame);
                if(ret){
                    delete frame;
                    break;
                }

                frameList.insertBack(frame);
            }
        }
        
        return 0;
    }
}