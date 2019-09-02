
#include "TechTechTechnologies.hpp"

typedef struct {
    unsigned short left;
    unsigned short _;
    unsigned short right;
    unsigned short value;

} IndexEntry;

struct Prometheus2 : Module {
    /* +ENUMS */
    #include "Prometheus2_enums.hpp"
    /* -ENUMS */
    
    /* +TRIGGER_VARS */
    #include "Prometheus2_vars.hpp"
    /* -TRIGGER_VARS */

    unsigned char* raw_index[2];
    unsigned short get_taps(unsigned short length, unsigned short param0, unsigned short param1, unsigned char order);

    Prometheus2()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        /* +CONFIGS */
        #include "Prometheus2_configs.hpp"
        /* -CONFIGS */

        for(int i = 0; i < 2; ++i)
        {
            char base_filename[256];
            sprintf(base_filename, "res/lookup_inv_%i.taps",i);
            
            FILE* fp = fopen(asset::plugin(pluginInstance, base_filename).c_str(), "rb");
            if(fp != 0)
            {
                fseek(fp, 0, SEEK_END);
                unsigned int length = ftell(fp);
                rewind(fp);
               
                raw_index[i] = new unsigned char[length];
                fread(raw_index[i], 1, length, fp);

                fclose(fp);

                get_taps(1, 30000, 30000, i);
                get_taps(11, 30000, 30000, i);
                get_taps(4095, 30000, 30000, i);

            }
            else
            {
                printf("ERROR: Couldn't find required file %s\n", base_filename);
            }
        }

    }
    void step() override;

    // For more advanced Module features, read Rack's engine.hpp header file
    // - toJson, fromJson: serialization of internal data
    // - onSampleRateChange: event triggered by a change of sample rate
    // - onReset, onRandomize, onCreate, onDelete: implements special behavior when user clicks these from the context menu
};

unsigned short Prometheus2::get_taps(unsigned short length, unsigned short param0, unsigned short param1, unsigned char order)
{
    unsigned char* buffer = raw_index[order];
    unsigned int base_address = ((unsigned int*)(buffer))[length];


    unsigned short actual_size = *(unsigned short*)(buffer+base_address-2);

//printf("Length %i maps to actual length %i starting at address %i\n", length, actual_size, base_address);

    unsigned short left0 = 0;
    unsigned short right0 = *(unsigned short*)(buffer+base_address+2);
    unsigned short index0 = 0;

//printf("There are %i options available here\n", right0);

    //I know there is a better way to do this, but I don't have bw to care rn.
    while(1)
    {
        IndexEntry* entry0 = (IndexEntry*)(buffer+base_address+index0*4);
        printf("%i ?? %i ?? %i\n", entry0->left, param0, entry0->right);
        printf("=%i/%i/%i\n", left0, index0, right0);

        if(param0 < entry0->left)
        {
            right0=index0;
            index0 = (index0+left0)/2;
        }
        else if(param0 >= entry0->right)
        {
            left0=index0;
            index0 = (index0+right0)/2;
        }
        else
        {
            unsigned int sub_address = base_address+*(unsigned short*)(buffer+base_address+6+index0*4);
            unsigned short left1 = 0;
            unsigned short right1 = *(unsigned short*)(buffer+sub_address+2);
            unsigned short index1 = 0;

//printf("  There are %i options available here\n", right1);

           
            unsigned int count = 0;
            while(count < 100)
            {
                count += 1;
                IndexEntry* entry1 = (IndexEntry*)(buffer+sub_address+index1*4);
//printf("  %i ?? %i ?? %i\n", entry1->left, param1, entry1->right);
//printf("  =%i/%i/%i\n", left1, index1, right1);
                if(param1 < entry1->left)
                {
                    right1 = index1;
                    index1 = (index1+left1)/2;
                }
                else if(param1 >= entry1->right)
                {
                    left1 = index1;
                    index1 = (index1+right1)/2;
                }
                else
                {
//printf("<----%x\n", entry1->value);
                    return entry1->value;
                }
            }
            return 0;
            
        }
    }

    
}



void Prometheus2::step() {
    float deltaTime = engineGetSampleTime();

    /*  +INPUT_PROCESSING */
    #include "Prometheus2_inputs.hpp"
    /*  -INPUT_PROCESSING */



    /*  +OUTPUT_PROCESSING */
    #include "Prometheus2_outputs.hpp"
    /*  -OUTPUT_PROCESSING */



}


struct Prometheus2Widget : ModuleWidget {

    Prometheus2Widget(Prometheus2 *module) {
        setModule(module);
        box.size = Vec(8.0 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Prometheus2.svg")));

        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));



        /* +CONTROL INSTANTIATION */
        #include "Prometheus2_panel.hpp"
        /* -CONTROL INSTANTIATION */
    }
};

#include "Prometheus2_instance.hpp"

