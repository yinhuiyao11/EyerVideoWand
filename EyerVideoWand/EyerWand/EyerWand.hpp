#ifndef	EYER_LIB_AV_WAND_H
#define	EYER_LIB_AV_WAND_H

#include "EyerGL/EyerGL.hpp"
#include "EyerGLWindow/EyerGLWindow.hpp"
#include "EyerCore/EyerCore.hpp"
#include "EyerAV/EyerAV.hpp"
#include "EyerGLCustomComponent/EyerGLCustomComponent.hpp"

#define EYER_WAND_VERSION "EyerWand 1.0.0"

namespace Eyer {
    class EyerWand;
    class EyerWandDebug;

    class EyerWandResourse;

    class EyerWandVideoResource;
    class EyerWandAudioResource;

    class EyerTrack;
    class EyerVideoTrack;
    class EyerVideoLayout;
    class EyerWandBuilder;

    class EyerVideoFragment;
    class EyerAudioFragment;

    class EyerVideoDecoderLine;
    class EyerTransKey;

    class EyerAudioLayer;

    class EyerWand {
    public:
        EyerWand();

        ~EyerWand();
    };

    class EyerWandResource {
    public:
        EyerString resPath;
    public:
        EyerWandResource();

        ~EyerWandResource();

        int SetPath(EyerString resPath);
    };

    class EyerVideoDecoderLine
    {
    public:
        EyerVideoDecoderLine(EyerString resPath, double initStart, EyerAVStreamType type = EyerAVStreamType::STREAM_TYPE_VIDEO);
        ~EyerVideoDecoderLine();

        int GetFrame(EyerAVFrame & frame, double ts);

        double GetStartTime();
    private:
        int ReadFrame();
        int SelectFrameInList(EyerAVFrame * & frame, double ts);

        int fileEndFlag = 0;

        double initStart = 0.0;
        EyerString resPath;

        EyerAVReader * reader = nullptr;
        EyerAVDecoder * decoder = nullptr;
        int videoStreamIndex = -1;

        EyerLinkedList<EyerAVFrame *> frameList;
    };

    class EyerWandVideoResource : public EyerWandResource {
    public:
        EyerWandVideoResource();
        ~EyerWandVideoResource();

        int GetVideoDuration(double & duration);
        int GetVideoFrame(EyerAVFrame & avFrame, double ts);

    private:
        EyerLinkedList<EyerVideoDecoderLine *> decoderLineList;
    };

    class EyerWandAudioResource : public EyerWandResource {
    public:
        EyerWandAudioResource();
        ~EyerWandAudioResource();
    };

    enum EyerVideoPanelType
    {
        YUV_DATA_PLANE,
        SINGLE_CHANNEL_PLANE
    };

    class EyerVideoPanel
    {
    public:
        EyerVideoPanel();
        ~EyerVideoPanel();

        EyerVideoPanel(const EyerVideoPanel & panel);
        EyerVideoPanel & operator = (const EyerVideoPanel & panel);

        int GetYDate(unsigned char * yData);
        int GetUDate(unsigned char * uData);
        int GetVDate(unsigned char * vData);

        int SetData(EyerAVFrame & frame);

        int GetW();
        int GetH();


        EyerGLTexture targetTexture;


        EyerMat4x4 GetMVPMat();

        EyerMat4x4 mvp;
    private:
        EyerAVFrame frame;
    };


    class EyerTrack {
    public:
        virtual ~EyerTrack()
        {

        }
    };

    /**
     * 视频轨
     */

    class EyerVideoTrackRenderParams
    {
    public:
        int videoW = 0;
        int videoH = 0;
        Eyer::EyerGLFrameBuffer * frameBuffer = nullptr;
        Eyer::EyerGLTextDraw * titleTextDraw = nullptr;
    };

    /**
     * 视频轨
     */
    class EyerVideoTrack : public EyerTrack {
    public:
        EyerVideoTrack();

        ~EyerVideoTrack();

        EyerVideoTrack(const EyerVideoTrack &track);
        EyerVideoTrack & operator = (const EyerVideoTrack &track);

        int AddLayout(const EyerVideoLayout &layout);
        int AddLayer(const EyerVideoLayout &layout);

        int GetFrameCount();

        int RenderFrame(int frameIndex, EyerVideoTrackRenderParams * params, int fps);
    private:
        EyerLinkedList<EyerVideoLayout *> layoutList;
    };

    /**
     * 音频轨
     */
    class EyerAudioTrack : public EyerTrack {
    public:
        EyerAudioTrack();
        ~EyerAudioTrack();

        EyerAudioTrack(const EyerAudioTrack & track);
        EyerAudioTrack & operator = (const EyerAudioTrack &track);

        int AddLayer(const EyerAudioLayer & layer);

        double GetCountTime();

        int RenderFrame(double ts, EyerAVFrame & outFrame);
    private:
        EyerLinkedList<EyerAudioLayer *> layoutList;
    };

    class EyerAudioLayer {
    public:
        EyerAudioLayer();
        ~EyerAudioLayer();

        EyerAudioLayer(const EyerAudioLayer & layer);
        EyerAudioLayer & operator = (const EyerAudioLayer & layer);

        int RenderLayerFrame(double ts, EyerAVFrame & frame);

        int SetTime(double startTime, double endTime);

        int AddAudioFragment(const EyerAudioFragment & audio);

        int SetWeight(float _weight);
        float GetWeight();

        double GetStartTime();
        double GetEndTime();
    private:
        double startTime = 0.0;
        double endTime = 0.0;

        float weight = -1.0f;

        EyerLinkedList<EyerAudioFragment *> audioFragmentList;
    };

    class EyerVideoLayout {
    public:
        EyerVideoLayout();

        ~EyerVideoLayout();

        EyerVideoLayout(const EyerVideoLayout &layout);

        EyerVideoLayout & operator = (const EyerVideoLayout &layout);


        int AddVideoFragment(const EyerVideoFragment * fragment);

        int SetFrame(int startFrameIndex, int endFrameIndex);
        int GetStartFrameIndex();
        int GetEndFrameIndex();


        int GetVideoFragmentCount();
        int GetVideoPanel(EyerVideoPanel * panel, EyerVideoFragment ** fragmentP, int videoFragmentIndex, int layoutFrameIndex, int fps, EyerVideoTrackRenderParams * params);
    private:
        int startFrameIndex = 0;
        int endFrameIndex = 0;

        EyerLinkedList<EyerVideoFragment *> videoFragmentList;
    };

    class EyerAudioFragment
    {
    public:
        EyerAudioFragment();
        ~EyerAudioFragment();

        EyerAudioFragment(const EyerAudioFragment & fragment);
        EyerAudioFragment & operator = (const EyerAudioFragment & fragment);

        int SetWeight(float _weight);
        float GetWeight();

        int LoadAudioFile(EyerString path);

        int ReaderAVFrame(double ts, EyerAVFrame & frame);

    private:
        float weight = -1.0f;

        EyerString path;
        EyerVideoDecoderLine * decoderLine = nullptr;
    };

    class EyerTransKey
    {
    public:
        double t = 0.0;
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
    public:
        bool operator < (const EyerTransKey &key2);

        EyerTransKey();
        EyerTransKey(const EyerTransKey & tk);

        EyerTransKey & operator = (const EyerTransKey & tk);
    };

    enum EyerVideoFragmentType
    {
        VIDEO_FRAGMENT_VIDEO,
        VIDEO_FRAGMENT_TEXT
    };

    class EyerVideoFragment
    {
    public:
        virtual ~EyerVideoFragment();
        virtual EyerVideoFragmentType GetType() const = 0;

        static EyerVideoFragment * CopyFragment(const EyerVideoFragment * fragment);
    };

    enum EyerVideoChangeType
    {
        VIDEO_FRAGMENT_CHANGE_TRANS,
        VIDEO_FRAGMENT_CHANGE_SCALE
    };
    class EyerVideoFragmentVideo : public EyerVideoFragment
    {
    public:
        EyerVideoFragmentVideo();
        ~EyerVideoFragmentVideo();

        EyerVideoFragmentVideo(const EyerVideoFragmentVideo & fragment);
        EyerVideoFragmentVideo & operator = (const EyerVideoFragmentVideo & fragment);

        int LoadVideoFile(EyerString path);

        int SetFrameIndex(int startIndex, int endIndex);
        int SetFrameTime(double startTime, double endTime);

        int AddTransKey(double t, float x, float y, float z);
        int AddScaleKey(double t, float x, float y, float z);
        int GetLinearValue(EyerVideoChangeType type, double t, float & x, float & y, float & z);

        double GetDuration();

        int Print();

        EyerString GetPath();

        int GetVideoFrame(EyerAVFrame & avFrame, double ts);

        virtual EyerVideoFragmentType GetType() const;
    private:
        EyerString path;

        int startIndex = 0;
        int endIndex = 0;
        double startTime = 0.0;
        double endTime = 0.0;

        double duration = 0.0;

        Eyer::EyerWandVideoResource * videoResource = nullptr;

        Eyer::EyerLinkedList<EyerTransKey *> transKeyList;
        Eyer::EyerLinkedList<EyerTransKey *> scaleKeyList;

    };

    class EyerVideoFragmentText : public EyerVideoFragment
    {
    public:
        EyerVideoFragmentText();
        ~EyerVideoFragmentText();

        EyerVideoFragmentText(const EyerVideoFragmentText & vft);

        EyerVideoFragmentText & operator = (const EyerVideoFragmentText & vft);

        int SetFontPath(EyerString path);
        int SetText(EyerString text);
        int SetColor(float r, float g, float b);
        int SetSize(float size);
        int SetPos(float x, float y);

        int GetTextWidth();

        float GetColorR();
        float GetColorG();
        float GetColorB();

        float GetPosX();
        float GetPosY();

        float GetSize();

        virtual EyerVideoFragmentType GetType() const;

    public:
        EyerString fontPath;
        EyerString text;
        float r = 0.0f;
        float g = 0.0f;
        float b = 0.0f;
        float size = 100.0f;
        float posX = 0.0f;
        float posY = 0.0f;
    };


    class EyerWandBuilder
    {
    public:
        EyerWandBuilder(EyerString path);
        ~EyerWandBuilder();

        int SetVideoWH(int w, int h);
        int SetVideoFPS(int fps);

        int AddVideoTrack(const EyerVideoTrack & videoTrack);
        int AddAudioTrack(const EyerAudioTrack & audioTrack);

        int Process();

    private:
        int VideoProcess(EyerAVWriter * writer, EyerAVEncoder * encoder, int streamIndex, int debug = 0);
        int VideoTrackProcess(EyerAVWriter * writer, EyerAVEncoder * encoder, int streamIndex, int debug = 0);
        int AudioTrackProcess(EyerAVWriter * writer, EyerAVEncoder * encoder, int streamIndex, int debug = 0);
    private:
        EyerString path;

        int videoWidth = 0;
        int videoHeight = 0;
        int videoFps = 25;

        EyerVideoTrack videoTrack;
        EyerAudioTrack audioTrack;
    };


    typedef EyerVideoLayout EyerVideoLayer;

    class EyerWandDebug
    {
    public:
        static long long DecoderTime;
        static long long RenderTime;
    };
}

#endif
