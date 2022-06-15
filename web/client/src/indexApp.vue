<template>
  <div id="app">
    <keep-alive>
      <router-view class="main-width"></router-view>
    </keep-alive>
  </div>
</template>

<script>
export default {
  name: "App",
  components: {},
  data() {
    return {};
  },
  mounted() {
    // setInterval(() => {
    //   console.log("scrollHeight:", document.body.scrollHeight);
    //   console.log("clientHeight:", document.body.clientHeight);
    //   console.log("offsetHeight:", document.body.offsetHeight);
    // }, 2000);
  },
};
</script>
