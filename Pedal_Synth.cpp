#include <AuLib.h>
#include <MidiIn.h>
#include <Adsr.h>
#include <Delay.h>
#include <Instrument.h>
#include <Oscili.h>
#include <SoundOut.h>
#include <SigBus.h>
#include <Note.h>
#include <BlOsc.h>
#include <iostream>
#include <map>

using namespace AuLib;

class SineSynth : public Note{

    // DSP override
    virtual const SineSynth &dsp() {
        if (!m_env.is_finished())
        {
            int i = 0;
            for(Oscili& oscil : m_oscils)
            {
                int cc_val = m_ctrlnums[i];
                int indx = m_ccs[cc_val];
                double freq = m_cps * m_pfreq[indx];
                double detuned = freq * pow(2, (m_det_incr * i - m_detune)/1200.);
                m_bus(oscil(m_env(), detuned), .25, 0., false);
                i++;
            }
            set(m_bus);
            m_bus.clear();
        }else
            clear();
      return *this;
    }

    virtual void off_note(){
        m_env.release();
    }

    virtual void on_note(){
        m_amp = (m_vel / 128.) ;
        m_cps = 440. * pow(2., (m_num - 69.) / 12.); 
        m_env.reset(m_amp, 0.1, 0.3, 0.6, 1.);
    }

    virtual void on_msg(uint32_t msg, const std::vector<double> &data,
        uint64_t tstamp){

            if(msg == midi::ctrl_msg)
            {
                //if incoming cc num matches saved cc, save its data
                uint32_t num = (uint32_t)data[0];
                m_ccs[num] = (data[1] / 127.) * 4.;
                if(num == m_ctrlnums[4])
                {
                    m_detune = m_ccs[num]/4. * m_max_detune_cents;
                    m_det_incr = (m_detune * 2)/3.;
                }
            }

        };

    protected:
    std::vector<Oscili> m_oscils;
    const double m_pfreq[5] = {1, 1.5, 2, 3, 4};
    Adsr   m_env;
    SigBus m_bus;
    
    double m_amp;
    double m_cps;

    //array to hold cc number data
    std::array<int, 5> m_pctrl;
    //array holds oscillator cps value
    std::array<int, 5> m_ctrlnums;
    //map holds 
    std::map<int, double> m_ccs;

    //detune amount in cents
    double m_detune;
    double m_det_incr;
    const double m_max_detune_cents;
    
    public:
    SineSynth(uint32_t chn, std::array<int , 5> ccs, std::array<double, 4> adsr) : Note(chn),
    m_env(0.9, adsr[0], adsr[1], adsr[2], adsr[3]),
    m_ctrlnums({ccs[0], ccs[1], ccs[2], ccs[3], ccs[4]}),
    m_ccs({{ccs[0], 0.}, {ccs[1], 0.}, {ccs[2], 0.}, {ccs[3], 0.}, {ccs[4], 0.}}),
    m_detune(0.), m_det_incr(0.), m_max_detune_cents(12.)
    {
        m_env.release();
        m_oscils.resize(4);
    };
};

class TriSynth : public SineSynth{

    // DSP override
    virtual const SineSynth &dsp() {
        if (!m_env.is_finished())
        {
            int i = 0;
            for(Oscili& oscil : m_oscils)
            {
                int cc_val = m_ctrlnums[i];
                int indx = m_ccs[cc_val];
                double freq = m_cps * m_pfreq[indx];
                double detuned = freq * pow(2, (m_det_incr * i - m_detune)/1200.);
                m_bus(oscil(m_env(), detuned), .25, 0., false);
                i++;
            }
            set(m_bus);
            m_bus.clear();
        }else
            clear();
      return *this;
    }

    std::vector<TriOsc> m_oscils;

    public:
        TriSynth(uint32_t chn, std::array<int, 5> ccs, std::array<double, 4> adsr) : SineSynth(chn, ccs, adsr) {
            m_env.release();
            m_oscils.resize(4);
        };

};


static std::atomic_bool running(true);
void signal_handler(int signal){
    running = false;
    std::cout << "\nexiting\n";
}


int main(int argc, char** argv)
{
    MidiIn midi;
    //MIDI cc values are determined in this array, change for your MIDI controller
    std::array<int, 5> MIDI_vals = {22, 23, 24, 25, 26};
    int del_CC = 27;
    //Pre-set ADSR values
    std::array<double, 4> ADSR_vals = {0.05, .3, .5, 1};
    
    double delay_amt;
    const double delay_fdbk = 0.7;
    const double delay_time = 1.;

    Instrument<SineSynth, std::array<int, 5>, std::array<double, 4>> Synth(8, 0, MIDI_vals, ADSR_vals);
    Instrument<TriSynth, std::array<int, 5>, std::array<double, 4>> TriSynth(8, 0, MIDI_vals, ADSR_vals);
    SigBus mix;
    Delay del(delay_time, delay_fdbk);
    AudioBase Buffer;

    

    int MIDI_tag;
    std::vector<std::string> MIDI_List;

    SoundOut output("dac", 1, def_vframes, def_sr);

    if(output.error() != AULIB_NOERROR)
    {
        std::cout << "error opening output\n";
        return -1;
    }


    MIDI_List = midi.device_list();

    std::cout << "Please select a MIDI device from the list below..." << std::endl;
    for(std::string string : MIDI_List)
        std::cout << string << std::endl;
    
    std::cin >> MIDI_tag;

    if (midi.open(MIDI_tag) == AULIB_NOERROR)
    {
            while(running)
            {
                Buffer = midi.listen(TriSynth);
                delay_amt = midi.ctlval(0, del_CC);
                mix(del(Buffer), delay_amt);
                Buffer += mix;
                output(Buffer);
                mix.clear();
            }
            midi.close();
    }else   
        std::cout << "Error Opening MIDI Device\nClosing Program..." << std::endl;

    return 1;
}
