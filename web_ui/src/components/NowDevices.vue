<template>
    <div>
        <div class="content">
            <span></span>
            <ui-button @click="addDevice()">Add new devices</ui-button>
        </div>

        <div class="container">
            <div v-for="(device, mac, index) in getNowDevices" class="content" :class="{ odd: !index%2, even: index%2 }" :key="mac">
                <span class="mac" v-html="mac"></span>
                <span class="spacer"></span>
                <span class="name">{{ device }}</span>
                <span class="spacer"></span>
                <ui-button small @click="editDevice(mac, device)">&#9998;</ui-button>
                <ui-button small @click="deleteDevice(mac, device)" color="red">&times;</ui-button>
            </div>
        </div>
        <modal :title="modalTitle" :show="modalShow" @close="modalClose()">
            <div class="rows">
                <mac-input v-model="editMac"></mac-input>
                <span class="spacer"></span>
                <ui-input v-model="editName" placeholder="Device name"></ui-input>
                <span class="spacer"></span>
                <div class="content">
                    <ui-button @click="modalClose()">Cancel</ui-button>
                    <ui-button @click="actionSave()" color="blue">OK</ui-button>
                </div>
            </div>
        </modal>
        <modal title="Delete NOW device?" :show="modalDeleteShow" @close="modalDeleteClose()">
            <div class="rows">
                <span>
                    Are you sure you what to delete device <b>{{editMac}}</b> with name <b>"{{editName}}"</b>?
                </span>
            </div>
            <template slot="footer">
                <div class="content">
                    <ui-button @click="modalDeleteClose()">Cancel</ui-button>
                    <ui-button @click="actionDelete()" color="red">Delete</ui-button>
                </div>
            </template>
        </modal>
    </div>
</template>

<script>
    import {mapGetters} from "vuex";

    export default {
        name: 'NowDevices',
        data() {
            return {
                modalTitle: '',
                modalShow: false,
                modalDeleteShow: false,
                selectedMac: undefined,
                editMac: '',
                editName: '',
            };
        },
        computed: {
            ...mapGetters({
                getNowDevices: 'nowDevices/getNowDevices',
            }),
        },
        methods: {
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
                this.$store.dispatch('nowDevices/remove', this.selectedMac)
                    .then(() => this.modalDeleteClose());
            },
            actionSave() {
                this.$store.dispatch('nowDevices/save', {
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
            .name {
                min-width: 0;
                white-space: nowrap;
                overflow: hidden;
                text-overflow: ellipsis;
                flex: 1 1 auto;
            }
            .mac {
                flex: 0 0 auto;
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
