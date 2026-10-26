import { nextTick, onMounted, watch } from 'vue'
import { useRoute } from 'vitepress'
import DefaultTheme from 'vitepress/theme'
import './custom.css'

// Adds drag-to-move and zoom to every Mermaid diagram on the page.
// Diagrams are drawn in the browser after the page loads (and again when the colour theme changes),
// so a MutationObserver keeps looking for new ones.
async function enhanceDiagrams() {
  const svgs = Array.from(document.querySelectorAll<SVGSVGElement>('.mermaid svg'))
  if (!svgs.length) return
  // @ts-ignore (no type declarations needed for this small library)
  const { default: svgPanZoom } = await import('svg-pan-zoom')

  for (const svg of svgs) {
    if (svg.dataset.panzoom) continue
    const box = svg.getBoundingClientRect()
    if (!box.width || !box.height) continue // not visible yet, try again on the next change
    svg.dataset.panzoom = '1'

    try {
      const pz = svgPanZoom(svg, {
        zoomEnabled: true,
        panEnabled: true,
        controlIconsEnabled: true,
        fit: true,
        center: true,
        minZoom: 0.4,
        maxZoom: 12,
        zoomScaleSensitivity: 0.3,
        mouseWheelZoomEnabled: false, // plain scrolling must still scroll the page
        dblClickZoomEnabled: true,
      })

      // Ctrl (or Cmd) + scroll zooms at the pointer
      svg.addEventListener(
        'wheel',
        (e: WheelEvent) => {
          if (!e.ctrlKey && !e.metaKey) return
          e.preventDefault()
          const r = svg.getBoundingClientRect()
          pz.zoomAtPointBy(e.deltaY < 0 ? 1.25 : 0.8, { x: e.clientX - r.left, y: e.clientY - r.top })
        },
        { passive: false },
      )

      window.addEventListener('resize', () => {
        pz.resize()
        pz.fit()
        pz.center()
      })

      const holder = svg.closest('.mermaid')
      if (holder && !holder.nextElementSibling?.classList.contains('diagram-hint')) {
        const hint = document.createElement('p')
        hint.className = 'diagram-hint'
        hint.textContent = 'Drag to move. Use + and - to zoom, or Ctrl + scroll. Double-click to zoom in.'
        holder.insertAdjacentElement('afterend', hint)
      }
    } catch {
      delete svg.dataset.panzoom // leave the plain diagram in place if the library can't attach
    }
  }
}

export default {
  extends: DefaultTheme,
  setup() {
    const route = useRoute()
    onMounted(() => {
      enhanceDiagrams()
      let timer: number | undefined
      new MutationObserver(() => {
        window.clearTimeout(timer)
        timer = window.setTimeout(enhanceDiagrams, 150)
      }).observe(document.body, { childList: true, subtree: true })
    })
    watch(
      () => route.path,
      () => nextTick(enhanceDiagrams),
    )
  },
}
