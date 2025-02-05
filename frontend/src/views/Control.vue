<template>
   
        <!-- HTML HERE -->    
        <!--<h2>New Template</h2>-->
    <v-container fluid class="d-flex justify-center align-center">
      <v-row class="ma-0" style="max-width: 1200px;">
         <!-- First Column -->
         <v-col cols="12" md="6" class="d-flex align-center justify-center">
           <!-- First Sheet -->
           <v-sheet color="surface" elevation="0" max-width="800" width="100%" class="mb-1 rounded-t-lg">
             <v-card class="text-secondary" title="LED Controls" color="surface" subtitle="Recent settings" variant="tonal" flat></v-card>
           </v-sheet>
            <!-- Second Sheet -->
           <v-sheet color="surface" elevation="0" max-width="800" width="100%" class="mb-1">
             <v-card class="pt-5" color="surface" variant="tonal">
               <v-slider
                v-model="led.brightness"
                class="pt-2 bg-surface"
                append-icon="mdi:mdi-car-light-high"
                density="compact"
                thumb-size="16"
                color="secondary"
                label="Brightness"
                direction="horizontal"
                min="0"
                max="250"
                step="10"
                show-ticks
                thumb-label="always">
               </v-slider>
             </v-card>
            </v-sheet>
             <!-- Third Sheet -->
           <v-sheet color="surface" elevation="0" max-width="800" width="100%" class="mb-1 d-flex justify-center align-center">
            <v-card class="pt-5" color="surface" variant="tonal">
             <v-slider
               v-model="led.leds"
               class="pt-2 bg-surface"
               append-icon="mdi:mdi-led-on"
               density="compact"
               thumb-size="16"
               color="secondary"
               label="LED Nodes"
               direction="horizontal"
               min="1"
               max="7"
               step="1"
               show-ticks
               thumb-label="always">
             </v-slider>
            </v-card>
           </v-sheet>
            <!-- Fourth Sheet -->
           <v-sheet color="surface" elevation="0" max-width="800" width="100%" class="mb-1 pa-2 d-flex justify-center align-center border">
            <v-progress-circular :model-value="led.leds * 15" :color="indicatorColor" rotate="0" size="200" width="15">
                <span class="text-onSurface font-weight-bold">{{ led.leds }} LED(s)</span>
            </v-progress-circular>
           </v-sheet>
         </v-col>

         <!-- Second Column -->
         <v-col cols="12" md="6" class="d-flex align-center justify-center">
            <v-color-picker v-model="led.color" />
         </v-col>
       </v-row>
    </v-container>
</template>

<script setup>
/** JAVASCRIPT HERE */

// IMPORTS
import { ref,reactive,watch ,onMounted,onBeforeUnmount,computed } from "vue";  
import { useRoute ,useRouter } from "vue-router";
import { useMqttStore } from '@/store/mqttStore'; // Import Mqtt Store
import { storeToRefs } from "pinia";
 
// VARIABLES
const router      = useRouter();
const route       = useRoute();  
const Mqtt = useMqttStore();
const { payload, payloadTopic } = storeToRefs(Mqtt);
const led = reactive({"brightness":255,"leds":1,"color":{ r: 255, g: 0, b: 255, a: 1 }});
let timer, ID = 1000;

// COMPUTED PROPERTIES
const indicatorColor = computed(()=>{
return `rgba(${led.color.r},${led.color.g},${led.color.b},${led.color.a})`
})

// FUNCTIONS
onMounted(()=>{
    // THIS FUNCTION IS CALLED AFTER THIS COMPONENT HAS BEEN MOUNTED
    Mqtt.connect(); // Connect to Broker located on the backend

    setTimeout( ()=>{
    // Subscribe to each topic
    Mqtt.subscribe("620162321");
    Mqtt.subscribe("620162321_sub","/elet2415");
    },3000);
});


onBeforeUnmount(()=>{
    // THIS FUNCTION IS CALLED RIGHT BEFORE THIS COMPONENT IS UNMOUNTED
    Mqtt.unsubcribeAll();
});

// WATCHERS
watch(led,(controls)=>{
clearTimeout(ID);
ID = setTimeout(()=>{
const message = JSON.stringify({"type":"controls","brightness":controls.brightness,"leds":controls.leds,"color":
controls.color});
Mqtt.publish("620162321_sub",message); // Publish to a topic subscribed to by the hardware
},1000)
})

</script>


<style scoped>
/** CSS STYLE HERE */


</style>
  