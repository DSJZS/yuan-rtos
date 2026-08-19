<script setup lang="ts">
import Giscus from '@giscus/vue'
import { useData } from 'vitepress'

const { isDark, page } = useData()
</script>

<template>
  <div class="giscus-wrapper">
    <hr class="giscus-divider" />
    <Giscus
      :key="page.relativePath"
      repo="DSJZS/yuan-rtos"
      repo-id="R_kgDOR-1uFg"
      category="Announcements"
      category-id="DIC_kwDOR-1uFs4DDtAC"
      mapping="pathname"
      strict="0"
      reactions-enabled="1"
      emit-metadata="0"
      input-position="bottom"
      :theme="isDark ? 'dark' : 'light'"
      lang="zh-CN"
      loading="lazy"
    />
  </div>
</template>

<style scoped>
.giscus-divider {
  margin: 32px 0;
  border: 0;
  border-top: 1px solid var(--vp-c-divider);
}
</style>
