import Vue from 'vue'
import App from './App.vue'
import store from './store'

import UiButton from "./components/UiButton";
import UiInput from "./components/UiInput";
import UiProgressbar from "./components/UiProgressbar";
import MacInput from "./components/MacInput";
import Modal from "./components/Modal";

Vue.component('ui-button', UiButton);
Vue.component('ui-input', UiInput);
Vue.component('ui-progressbar', UiProgressbar);
Vue.component('mac-input', MacInput);
Vue.component('modal', Modal);

Vue.config.productionTip = false;

const WS_URL = 'ws://now-gw.local/ws';

new Vue({
    store,
    created() {
        let url = WS_URL;
        if (process.env.NODE_ENV === 'production') {
            url = 'ws://' + location.host + '/ws';
        }
        const websocket = new WebSocket(url);
        websocket.onopen = (event) => store.dispatch('webSocketStatus', true);
        websocket.onclose = (event) => store.dispatch('webSocketStatus', false);
        websocket.onmessage = (event) => store.dispatch('webSocketData', event);
        websocket.onerror = (event) => store.dispatch('webSocketError', event);

        Vue.prototype.$ws = websocket;

        this.$store.dispatch('nowDevices/fetch');
    },
    render: h => h(App)
}).$mount('#app');

