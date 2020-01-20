<template>
    <div class="ui-progressbar">
        <div :style="'width: ' + percent"></div>
    </div>
</template>

<script>

    export default {
        name: 'UiProgressbar',
        data() {
            return {
            };
        },
        props: {
            percent: {
                type: String,
            },
        },
    };
</script>

<style scoped lang="scss">
    .ui-progressbar {
        display: inline-block;
        margin: 1px 2px;
        min-width: 50px;
        height: 10px;
        border: 1px solid lightgray;
        border-radius: 5px;
        vertical-align: baseline;
        overflow: hidden;
        div {
            height: 100%;
            background-color: #458CFF;
        }
    }
</style>
