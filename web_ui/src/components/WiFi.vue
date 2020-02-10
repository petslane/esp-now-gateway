<template>
    <div>
        <h4>Access Point</h4>
        <small><i>AP will be enabled only on missing WiFi connection</i></small>
        <div class="content">
            <ui-input placeholder="AP name" class="content" v-model="apNameInput" />
            <span class="spacer"></span>
            <ui-button @click="apSave()">Save</ui-button>
        </div>
        
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
    import {mapGetters, mapState} from "vuex";

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
                apNameInput: '',
            };
        },
        computed: {
            ...mapState({
                current: 'currentWifiNetwork',
                current_connected: 'currentWifiConnected',
                current_error: 'currentWifiError',
                scanning: 'scanningWifiNetworks',
                networks: 'wifiNetworks',
                apName: 'apName',
            }),
        },
        mounted() {
            this.$store.dispatch('getWifiSettings', this.selectedMac);
        },
        watch: {
            apName(value) {
                this.apNameInput = value;
            }
        },
        methods: {
            scan() {
                this.$store.dispatch('scanWifiNetworks');
            },
            apSave() {
                this.$store.dispatch('saveAP', this.apNameInput);
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
                const status = await this.$store.dispatch('connectWifi', {
                    ssid: ssid,
                    pass: pass,
                });

                if (status) {
                    this.modalReconnecting = false;
                    this.$store.commit('setCurrentWifiNetwork', ssid);
                } else if (status === null) {
                    this.connectingToWifiError = 'Unable to connect to device or timeout';
                } else if (status === false) {
                    this.connectingToWifiError = 'Unable to connect to WIFI. Wrong password or WIFI network not reachable.';
                }
            },
            modalClose() {
                this.modalShow = false;
            },
            modalDeleteClose() {
                this.modalDeleteShow = false;
            },
            addDevice() {
                this.modalTitle = 'Add new device';
                this.editMac = '';
                this.editName = '';
                this.selectedMac = undefined;
                this.modalShow = true;
            },
            editDevice(mac, name) {
                this.modalTitle = 'Edit device';
                this.editMac = mac;
                this.editName = name;
                this.selectedMac = mac;
                this.modalShow = true;
            },
            deleteDevice(mac, name) {
                this.editMac = mac;
                this.editName = name;
                this.selectedMac = mac;
                this.modalDeleteShow = true;
            },
            actionDelete() {
                this.$store.dispatch('deleteDevice', this.selectedMac)
                    .then(() => this.modalDeleteClose());
            },
            actionSave() {
                this.$store.dispatch('saveDevice', {
                    id: this.selectedMac,
                    mac: this.editMac,
                    name: this.editName,
                })
                    .then(() => this.modalClose())
                    .catch((err) => alert(err.message));
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
        height: 300px;
        border: 1px solid lightgray;

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
