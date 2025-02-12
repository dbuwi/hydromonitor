<template>
      <v-container fluid class="bg-surface d-flex justify-center">
        <v-row class="mx-auto" style="max-width: 1200px;">
            <!-- First Row -->
            <v-col cols="9">
                <figure class="highcharts-figure">
                    <div id="container"></div>
                </figure>
            </v-col>
            <v-col cols="3">
                <!-- Sidebar content -->
                <v-card class="pa-4">
                    <v-card-text>
                        <v-sheet class="pa-2 mb-2 text-center" color="blue-lighten-4">
                            <strong>Temperature</strong>
                            <div class="text-h4">30.50 °C</div>
                        </v-sheet>
                        <v-sheet class="pa-2 mb-2 text-center" color="pink-lighten-4">
                            <strong>Heat Index (Feels like)</strong>
                            <div class="text-h4">34.15 °C</div>
                        </v-sheet>
                        <v-sheet class="pa-2 text-center" color="blue-lighten-4">
                            <strong>Humidity</strong>
                            <div class="text-h4">61.50 %</div>
                        </v-sheet>
                    </v-card-text>
                </v-card>

                 
                 <v-col cols="12" >
                  <figure class="highcharts-figure">
                    <div id="container"></div>
                  </figure>
                 </v-col>
            </v-col>
        </v-row>

        <v-row class="mx-auto justify-start" style="max-width: 1200px;">
            <!-- Second Row -->
            <v-col cols="9">
                <figure class="highcharts-figure">
                    <div id="container1"></div>
                </figure>
            </v-col>
        </v-row>
    </v-container>
</template>

<script setup>
/** JAVASCRIPT HERE */

// IMPORTS
import { ref,reactive,watch ,onMounted,onBeforeUnmount,computed } from "vue";  
import { useRoute ,useRouter } from "vue-router";
// Highcharts, Load the exporting module and Initialize exporting module.
import Highcharts from 'highcharts';
import more from 'highcharts/highcharts-more';
import Exporting from 'highcharts/modules/exporting';
import { useMqttStore } from "@/store/mqttStore";
import { storeToRefs } from "pinia";
Exporting(Highcharts);
more(Highcharts);
 
// VARIABLES
const router      = useRouter();
const route       = useRoute();  
const Mqtt        = useMqttStore();
const {payload, payloadTopic } = storeToRefs(Mqtt);
const tempHiChart = ref(null); // Chart object

const CreateCharts = async () => {
// TEMPERATURE CHART
tempHiChart.value = Highcharts.chart('container', {
chart: { zoomType: 'x' },
title: { text: 'Air Temperature and Heat Index Analysis', align: 'left' },
yAxis: {
title: { text: 'Air Temperature & Heat Index' , style:{color:'#000000'}},
labels: { format: '{value} °C' }
},
xAxis: {
type: 'datetime',
title: { text: 'Time', style:{color:'#000000'} },
},
tooltip: { shared:true, },
series: [
{
name: 'Temperature',
type: 'spline',
data: [],
turboThreshold: 0,
color: Highcharts.getOptions().colors[0]
},
{
name: 'Heat Index',
type: 'spline',
data: [],
turboThreshold: 0,
color: Highcharts.getOptions().colors[1]
} ],
});
};

const points = ref(10); // Specify the quantity of points to be shown on the live graph simultaneously.
const shift = ref(false); // Delete a point from the left side and append a new point to the right side of the graph.

// FUNCTIONS
onMounted(()=>{
// THIS FUNCTION IS CALLED AFTER THIS COMPONENT HAS BEEN MOUNTED
CreateCharts();
Mqtt.connect(); // Connect to Broker located on the backend
setTimeout( ()=>{
// Subscribe to each topic
Mqtt.subscribe("620162321");
Mqtt.subscribe("620162321_sub","/elet2415");
},3000);
});

onBeforeUnmount(()=>{
// THIS FUNCTION IS CALLED RIGHT BEFORE THIS COMPONENT IS UNMOUNTED
// unsubscribe from all topics
 Mqtt.unsubcribeAll();
});

// WATCHERS
watch(payload,(data)=> {
if(points.value > 0){ points.value -- ; }
else{ shift.value = true; }
tempHiChart.value.series[0].addPoint({y:parseFloat(data.temperature.toFixed(2)) ,x: data.timestamp * 1000 },
true, shift.value);
tempHiChart.value.series[1].addPoint({y:parseFloat(data.heatindex.toFixed(2)) ,x: data.timestamp * 1000 },
true, shift.value);
})

// COMPUTED PROPERTIES
const temperature = computed(()=>{
if(!!payload.value){
return `${payload.value.temperature.toFixed(2)} °C`;
}
});
 
const humidity = computed(()=>{
if(!!payload.value){
return `${payload.value.temperature.toFixed(2)} °C`;
}
});

const heatindex = computed(()=>{
if(!!payload.value){
return `${payload.value.temperature.toFixed(2)} °C`;
}
});
</script>


<style scoped>
/** CSS STYLE HERE */
.highcharts-figure {
    margin: 0 auto;
}
Figure {
border: 2px solid black;
}

</style>
  