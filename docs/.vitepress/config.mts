import { defineConfig } from 'vitepress'

// https://vitepress.dev/reference/site-config
export default defineConfig({
  lastUpdated: true,
  title: "Yuan RTOS",
  description: "一个轻量化的 RTOS 编写指南",
  // 设定 public 根目录
  base: '/yuan-rtos/',
  themeConfig: {
    search: {
      provider: 'local'
    },

    nav: [
      { text: '首页', link: '/' },
      { text: '前言', link: '/preface/' },
      { text: '简介', link: '/introduction/' },
    ],

    sidebar: [
      {
        text: '前言',
        items: [
          { text: '前言', link: '/preface/' },
        ]
      },
      {
        text: '项目简介',
        items: [
          { text: '简介', link: '/introduction/' },
          { text: '优缺点', link: '/introduction/advantages-and-disadvantages' },
          { text: '安装', link: '/introduction/installation' },
        ]
      },
      {
        text: 'RTOS 基础',
        items: [
          { text: '快速入门', link: '/rtos/' },
        ]
      },
      {
        text: 'Yuan RTOS 教程',
        items: [
          { text: '教程', link: '/tutorial/' },
          {
            text: 'API 手册',
            collapsed: true,
            items: [
              { text: '总览与约定', link: '/tutorial/api/' },
              { text: '调试方法', link: '/tutorial/api/debug' },
              { text: '数据结构', link: '/tutorial/api/list' },
              { text: '内核初始化与启动', link: '/tutorial/api/kernel' },
              { text: '移植接口', link: '/tutorial/api/porting' },
              { text: '任务管理', link: '/tutorial/api/task' },
              { text: '软件定时器', link: '/tutorial/api/timer' },
              { text: '调度器', link: '/tutorial/api/scheduler' },
              { text: 'IPC 基础', link: '/tutorial/api/ipc' },
              { text: '信号量', link: '/tutorial/api/semaphore' },
              { text: '互斥锁', link: '/tutorial/api/mutex' },
              { text: '队列', link: '/tutorial/api/queue' },
              { text: '函数速查表', link: '/tutorial/api/reference' },
              { text: '设计要点与局限', link: '/tutorial/api/design' },
            ],
          },
          {
            text: '移植手册',
            collapsed: true,
            items: [
              { text: '移植概览', link: '/tutorial/porting/' },
              { text: '目录组织', link: '/tutorial/porting/layout' },
              { text: '移植接口', link: '/tutorial/porting/interfaces' },
              { text: '任务切换与寄存器', link: '/tutorial/porting/context-switch' },
              { text: '系统 tick', link: '/tutorial/porting/tick' },
              { text: 'BSP 补充内容', link: '/tutorial/porting/bsp' },
              { text: '移植步骤', link: '/tutorial/porting/steps' },
              { text: '最小验证', link: '/tutorial/porting/verify' },
              { text: '常见错误排查', link: '/tutorial/porting/troubleshooting' },
              { text: '移植收尾', link: '/tutorial/porting/wrap-up' },
            ],
          },
        ]
      },
      {
        text: '常见问题 FAQ',
        items: [
          { text: '常见问题 FAQ', link: '/faq/' },
        ]
      }
    ],

    socialLinks: [
      { icon: 'github', link: 'https://github.com/DSJZS/yuan-rtos' }
    ],

    lastUpdated: {
      formatOptions: {
        dateStyle: 'short',
        timeStyle: 'short'
      }
    },

    editLink: {
      pattern: 'https://github.com/DSJZS/yuan-rtos/edit/main/docs/:path',
    },


  }
})
