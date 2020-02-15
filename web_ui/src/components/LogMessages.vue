<template>
    <div>
        <h4>Log</h4>
        <div class="content">
            <span>
                Status:
                <span :class="{ 'ws-connected': !!webSocketConnected, 'ws-disconnected': !webSocketConnected }">{{ webSocketConnected ? 'Connected' : 'Disconnected' }}</span>
            </span>
            <ui-button @click="clearMessages">Clear messages</ui-button>
        </div>

        <div class="log-message-container">
            <div v-for="(message, i) in webSocketMessages" :class="getLogMessageClass(message)" :key="i">
                <span class="log-message-datetime" v-html="getFormattedDateTime(message[1])"></span>
                <span v-if="message[0] === 'data'">{{ message[2].data }}</span>
                <span v-else-if="message[0] === 'in'">
                    <span style="display: inline-block">
                        Received from {{ message[2].from }}
                    </span>
                    <div class="log-message-now-data">{{ message[2].msg }}</div>
                </span>
                <span v-else-if="message[0] === 'out'">
                    <span style="display: inline-block">
                        Sent to {{ message[2].to }}
                        <small v-if="message[2].delivered">(delivered)</small>
                        <small v-else-if="message[2].delivered === false">(not delivered)</small>
                        <small v-else>(sending)</small>
                    </span>
                    <div class="log-message-now-data">{{ message[2].msg }}</div>
                </span>
            </div>
        </div>
    </div>
</template>

<script>
    import {mapGetters} from "vuex";
    import dateformat from 'dateformat';

    export default {
        name: 'LogMessages',
        data() {
            return {};
        },
        computed: {
            ...mapGetters({
                webSocketMessages: 'getWebSocketMessages',
                webSocketConnected: 'getWebSocketConnected',
            }),
        },
        methods: {
            getLogMessageClass(message) {
                let classes = {
                    'log-message': true,
                };
                if (message[0] === 'status') {
                    classes['log-message-status-connected'] = !!message[2];
                    classes['log-message-status-disconnected'] = !message[2];
                } else if (message[0] === 'in') {
                    classes['log-message-in'] = true;
                    classes['log-message-now-data'] = true;
                } else if (message[0] === 'out') {
                    classes['log-message-out'] = true;
                    classes['log-message-delivered'] = !!message[2].delivered;
                    classes['log-message-not-delivered'] = message[2].delivered === false;
                    classes['log-message-now-data'] = true;
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
            clearMessages() {
                this.$store.dispatch('clearLogMessages');
            },
        },
    };
</script>

<style scoped lang="scss">
    .content {
        & > * {
            flex: 0 1 auto;
        }
        flex: 0 0 auto;
    }

    .log-message-header {
        display: flex;
        justify-content: space-between;
    }

    .ws-disconnected {
        color: red;
    }

    .ws-connected {
        color: green;
    }

    .log-message-container {
        overflow-y: auto;
        height: 500px;
        border: 1px solid lightgray;
    }

    .log-message {
        border-top: 1px solid lightgray;
        padding: 3px 10px;

        &.log-message-status-connected {
            background-color: lightgreen;
        }
        &.log-message-in {
            background-color: rgb(180, 219, 255);
        }
        &.log-message-out {
            background-color: rgb(238, 240, 207);
        }
        &.log-message-out.log-message-delivered {
            background-color: rgb(196, 248, 128);
        }
        &.log-message-out.log-message-not-delivered {
            background-color: rgb(248, 200, 128);
        }
        &.log-message-status-disconnected {
            background-color: lightsalmon;
        }
        &.log-message-message {
            background-color: lightcyan;
        }
        .log-message-now-data {
            font-family: monospace;
        }
        .log-message-datetime {
            font-style: oblique;
            padding-right: 10px;
            font-size: 90%;
        }
    }
</style>
