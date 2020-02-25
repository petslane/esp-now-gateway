<template>
    <div>
        <br />
        <div class="content">
            <mac-input v-model="mac" class="mac" placeholder="To MAC address" />
            <span class="spacer"></span>
            <ui-input v-model="msg" class="msg" placeholder="Message"></ui-input>
            <span class="spacer"></span>
            <ui-button @click="send" :disabled="!webSocketConnected || mac.length < 17">Send</ui-button>
        </div>
    </div>
</template>

<script>
    import {mapGetters} from "vuex";
    import dateformat from 'dateformat';
    import ws from '../mixins/ws';
    import {WS_MSG_TYPE_SEND_MESSAGE} from "../constants";

    export default {
        mixins: [ws],
        name: 'send-now-message',
        data() {
            return {
                mac: '36:33:33:33:33:33',
                msg: 'Test message to send',
            };
        },
        computed: {
            ...mapGetters({
                webSocketMessages: 'getWebSocketMessages',
                webSocketConnected: 'logMessages/getConnected',
            }),
        },
        watch: {
            mac(value) {
                const valueNew = value.replace(/[^0-9a-fA-F]*/g, '').replace(/.{2}/g, '$&:').substring(0, 17);
                if (valueNew !== value) {
                    this.mac = valueNew;
                }
            },
        },
        methods: {
            getLogMessageClass(message) {
                let classes = {
                    'log-message': true,
                };
                if (message[0] === 'status') {
                    classes['log-message-status-connected'] = !!message[2];
                    classes['log-message-status-disconnected'] = !message[2];
                } else if (message[0] === 'error') {
                } else if (message[0] === 'data') {
                    classes['log-message-message'] = true;
                }
                return classes;
            },
            getFormattedDateTime(d) {
                let time = dateformat(d, 'HH:MM:ss');
                let millis = dateformat(d, 'l');
                let date = dateformat(d, 'dd.mm.yyyy');
                return `${time}.<span style="font-size: 70%">${millis}</span> ${date}`;
            },
            send() {
                this.sendWebSocketMessage(WS_MSG_TYPE_SEND_MESSAGE, this.mac, this.msg);
            },
        },
    };
</script>

<style scoped lang="scss">
    .content {
        .mac {
            flex: 0 0 135px;
        }
        button {
            flex: 0 0 auto;
        }
        .msg {
            flex: 1 0 80px;
        }
    }
</style>
