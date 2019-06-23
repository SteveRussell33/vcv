#include "TechTechTechnologies.hpp"
#define N 5
#define MIN(A,B) ((A<B)? A : B)
#define MAX(A,B) ((A>B)? A : B)
#define CLIP(A, B, C) MIN(MAX(A,B),C)

struct Ouroboros : Module {
	enum ParamIds {
        WAVE_PARAM,
        BASEFREQ_PARAM,
        FREQ_PARAM = BASEFREQ_PARAM+N,
        FREQCV_PARAM = FREQ_PARAM+N,
		NUM_PARAMS = FREQCV_PARAM+N
	};
	enum InputIds {
        BASEFREQ_INPUT,
        WAVE_INPUT,
        FREQ_INPUT = WAVE_INPUT+N,
		NUM_INPUTS = FREQ_INPUT+N
	};
	enum OutputIds {
        SIGNAL_OUTPUT,
		NUM_OUTPUTS=SIGNAL_OUTPUT+N
	};
	enum LightIds {
		NUM_LIGHTS
	};

    int ready = 0;

    Label* testLabel;

    double phase[N] = {0};

	Ouroboros() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;

	// For more advanced Module features, read Rack's engine.hpp header file
	// - dataToJson, dataFromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - onReset, onRandomize, onCreate, onDelete: implements special behavior when user clicks these from the context menu
};


void Ouroboros::step() {
  float deltaTime = 1.0 / engineGetSampleRate();

    if(ready == 0) return;
    
    double freq = inputs[BASEFREQ_INPUT].value + params[BASEFREQ_PARAM].value;
    freq = 261.626*pow(2, freq);

    double wave = inputs[WAVE_INPUT].value + params[WAVE_PARAM].value;

    for(int j = 0; j < N; ++j)
    {
        //Determine frequency

        double feff = params[FREQ_PARAM+j].value;
        double fscale = params[FREQCV_PARAM+j].value;
        if(inputs[FREQ_INPUT+j].active)
        {
            feff += fscale*inputs[FREQ_INPUT+j].value;
        }
        else
        {
            int idx = j-1;
            if(idx < 0) idx += N;
            feff += fscale*outputs[SIGNAL_OUTPUT+idx].value;
        }
        feff = freq*pow(2, feff);

        phase[j] += feff*deltaTime;
        if(phase[j] >= 1) phase[j] -=1;
    }

    wave =pow(2,wave);

    for(int j = 0; j < N; ++j)
    {
        double result = sin(6.28*phase[j]);

        if(result > 0)
            result = pow(result, 1/wave);
        else
            result = -pow(-result, 1/wave);

        outputs[SIGNAL_OUTPUT+j].value = 5*result;
 
    }

/*
            char tstr[256];
            sprintf(tstr, "%f, %f, %f", r, a, g);
//            sprintf(tstr, "%f, %e", norm, g);
            if(testLabel)
                testLabel->text = tstr;
*/

}

struct OuroborosWidget : ModuleWidget
{
    OuroborosWidget(Ouroboros* module);
};


OuroborosWidget::OuroborosWidget(Ouroboros* module) : ModuleWidget(module) {
//	Ouroboros *module = new Ouroboros();
//	setModule(module);
	box.size = Vec(12* RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(pluginInstance, "res/Ouroboros.svg")));
		addChild(panel);
	}

	addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));


    float xoff, yoff;

    xoff = 12.5;
    yoff = 380-302.5-20;

    addInput(createPort<PJ301MPort>(
        Vec(xoff, yoff), PortWidget::INPUT, module, Ouroboros::BASEFREQ_INPUT
        ));


    addParam(createParam<RoundLargeBlackKnob>(
        Vec(xoff+28.15, yoff-6.35), module, Ouroboros::BASEFREQ_PARAM,
        -2, 2, 0
        ));

    addInput(createPort<PJ301MPort>(
        Vec(xoff+28+44+40.5, yoff), PortWidget::INPUT, module, Ouroboros::WAVE_INPUT
        ));


    addParam(createParam<RoundLargeBlackKnob>(
        Vec(xoff+28.15+44, yoff-6.35), module, Ouroboros::WAVE_PARAM,
        0, 4, 0
        ));


    xoff = 12.5;
    yoff = 380-302.5-25+53;

    for(int j = 0; j < N; ++j)
    {

        CV_ATV_PARAM(xoff, yoff, Ouroboros::FREQ, -2,2,0,j)

        addOutput(createPort<PJ301MPort>(
            Vec(xoff+112.5, yoff), PortWidget::OUTPUT, module, Ouroboros::SIGNAL_OUTPUT+j
            ));


        yoff += 53;
    }


    auto* label = new Label();
    label->box.pos=Vec(0, 30);
    label->text = "";
    addChild(label); 
    module->testLabel = label;

    module->ready = 1;

}

    Model* modelOuroboros = createModel<Ouroboros, OuroborosWidget>(
    "Ouroboros", 
    );

