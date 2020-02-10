<template>
    <ui-input v-model="mac" placeholder="MAC address"></ui-input>
</template>

<script>
    export default {
        name: 'mac-input',
        data() {
            return {
                mac: this.value,
            };
        },
        props: {
            value: {
                type: String,
            },
        },
        watch: {
            value(value) {
                this.mac = value;
            },
            mac(value) {
                const valueNew = value.replace(/[^0-9a-fA-F]*/g, '').replace(/.{2}/g, '$&:').substring(0, 17);

                this.$nextTick(() => {
                    this.mac = valueNew;
                    this.$emit('input', valueNew)
                });
            },
        },
    };
</script>
