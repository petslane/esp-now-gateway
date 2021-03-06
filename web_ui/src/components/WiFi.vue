<template>
    <div>
        <h4>WiFi connection</h4>
        <div class="content">
            <span>
                Using <i>{{ current }}</i>
                &nbsp;
                <span v-if="current_connected">(connected)</span>
                <span v-if="!current_connected">({{ current_error }})</span>
            </span>
            <ui-button @click="scan()" :disabled="scanning">
                {{ scanning ? 'Scanning...' : 'Scan networks' }}
            </ui-button>
        </div>

        <div class="container">
            <div v-if="!networks.length" class="no-networks">No networks. Press 'Scan networks'</div>
            <div v-for="(network, index) in networks" class="content" :class="{ odd: !index%2, even: index%2 }" :key="`${index}::${network[0]}`">
                <span class="network">
                    &nbsp;
                    {{ network[3] ? '&#128274;' : '&#128275;' }}
                    {{ network[0] }}
                    <small>{{ network[2] }} dBm</small>
                </span>
                <span class="spacer"></span>
                <ui-button small @click="connectModal(network)" :disabled="scanning || current == network[0] && current_connected">
                    {{ current == network[0] && current_connected ? 'Connected' : 'Connect' }}
                </ui-button>
            </div>
        </div>
        <modal
            :closable="!!connectingToWifiError"
            title="WIFI"
            :show="modalReconnecting"
            :backgroundColor="connectingToWifiError ? '#f66' : undefined"
            @close="modalReconnecting = false"
        >
            <div class="rows">
                <span v-if="!connectingToWifiError">
                Connecting to <i>{{ connectingToWifi }}</i><br /><br />
                Please wait...
                </span>
                <span v-if="connectingToWifiError">{{ connectingToWifiError }}</span>
            </div>
        </modal>

        <modal
            title="WIFI"
            :show="modalConnect && connectToWifi"
            @close="modalConnect = false"
        >
            <div class="rows">
                <div>Connect to <i>{{ connectToWifi && connectToWifi[0] }}</i>?</div>
                <div class="spacer">&nbsp;</div>
                <div class="content">
                    <ui-input v-if="connectToWifi && connectToWifi[3]" v-model="connectPassword" placeholder="Password"></ui-input>
                    <div v-else class="spacer">&nbsp;</div>
                    <ui-button
                        :disabled="connectToWifi && connectToWifi[3] && !connectPassword"
                        @click="connect(connectToWifi && connectToWifi[0], connectPassword)"
                        class="right"
                        color="blue"
                    >
                        Connect
                    </ui-button>
                </div>
            </div>
        </modal>
    </div>
</template>

<script>
    import {mapState} from "vuex";

    export default {
        name: 'WiFi',
        data() {
            return {
                connectingToWifi: '',
                connectingToWifiError: '',
                modalReconnecting: false,
                modalConnect: false,
                connectToWifi: undefined,
                connectPassword: undefined,
            };
        },
        computed: {
            ...mapState('config', {
                current: 'currentWifiNetwork',
                current_connected: 'currentWifiConnected',
                current_error: 'currentWifiError',
                scanning: 'scanningWifiNetworks',
                networks: 'wifiNetworks',
            }),
        },
        mounted() {
            this.$store.dispatch('config/getWifiSettings', this.selectedMac);
        },
        methods: {
            scan() {
                this.$store.dispatch('config/scanWifiNetworks');
            },
            connectModal(network) {
                this.modalConnect = true;
                this.connectToWifi = network;
                this.connectPassword = undefined;
            },
            async connect(ssid, pass) {
                this.modalConnect = false;
                this.connectingToWifi = ssid;
                this.connectingToWifiError = '';
                this.modalReconnecting = true;
                const status = await this.$store.dispatch('config/connectWifi', {
                    ssid: ssid,
                    pass: pass,
                });

                if (status) {
                    this.modalReconnecting = false;
                    this.$store.commit('config/setCurrentWifiNetwork', ssid);
                } else if (status === null) {
                    this.connectingToWifiError = 'Unable to connect to device or timeout';
                } else if (status === false) {
                    this.connectingToWifiError = 'Unable to connect to WIFI. Wrong password or WIFI network not reachable.';
                }
            },
        },
    };
</script>

<style scoped lang="scss">
    .right {
        float: right;
    }

    .container {
        overflow-y: auto;
        border: 1px solid lightgray;
        margin-bottom: 5px;

        .no-networks {
            text-align: center;
            padding: 15px;
            color: #666;
        }
        .content {
            .network {
                flex: 0 0 auto;
                small {
                    color: #666;
                }
            }
            button {
                flex: 0 0 auto;
            }
            &.odd {
                background-color: #f5f5f5;
            }
            &.even {
                background-color: #fefefe;
            }
            &:hover {
                &.odd, &.even {
                    background-color: #fafafa;
                }
            }
        }
    }
</style>
