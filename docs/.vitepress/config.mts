import { defineConfig } from 'vitepress'
import { withMermaid } from 'vitepress-plugin-mermaid'

const repo = 'https://github.com/TheLlamainator/TI84-AI-Mod'

export default withMermaid(
  defineConfig({
    title: 'TI-84 AI Mod',
    titleTemplate: ':title | TI-84 AI Mod',
    head: [['link', { rel: 'icon', type: 'image/svg+xml', href: '/TI84-AI-Mod/favicon.svg' }]],
    description: 'WiFi, GPT chat, a camera and wireless updates for a TI-84 calculator, using an ESP32-S3 and a self-hosted server.',
    // GitHub Pages serves the site from /<repo-name>/
    base: '/TI84-AI-Mod/',
    cleanUrls: true,
    lastUpdated: true,
    themeConfig: {
      logo: '/favicon.svg',
      nav: [
        { text: 'Guide', link: '/overview' },
        { text: 'Build it', link: '/ordering-the-pcb' },
        { text: 'GitHub', link: repo },
      ],
      sidebar: [
        {
          text: 'Start here',
          items: [
            { text: 'Overview', link: '/overview' },
            { text: 'Downloads', link: '/downloads' },
          ],
        },
        {
          text: 'Build it',
          items: [
            { text: '1. Order the PCB', link: '/ordering-the-pcb' },
            { text: '2. Wiring and parts', link: '/wiring' },
            { text: '3. Server setup', link: '/server-setup' },
            { text: '4. Flash and use it', link: '/firmware-setup' },
          ],
        },
        {
          text: 'Reference',
          items: [
            { text: 'Credits and licence', link: '/credits' },
            { text: 'Original TI-32 docs', link: '/original-documentation' },
          ],
        },
      ],
      socialLinks: [{ icon: 'github', link: repo }],
      search: { provider: 'local' },
      editLink: { pattern: `${repo}/edit/main/docs/:path`, text: 'Edit this page on GitHub' },
      footer: { message: 'Released under the GNU GPL v3.', copyright: 'Based on the original TI-32 project.' },
    },
    mermaid: {},
  })
)
