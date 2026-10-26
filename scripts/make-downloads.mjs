// Builds the files offered on the Downloads page into docs/public/downloads/.
// Runs before the docs build (see package.json), so the downloads always match the repo.
import fs from 'node:fs'
import path from 'node:path'
import archiver from 'archiver'

const root = path.resolve(import.meta.dirname, '..')
const out = path.join(root, 'docs', 'public', 'downloads')
fs.rmSync(out, { recursive: true, force: true })
fs.mkdirSync(out, { recursive: true })

const copy = (from, to) => fs.copyFileSync(path.join(root, from), path.join(out, to))

// Files that must never end up in a download
const excluded = (rel) => {
  const parts = rel.split(path.sep)
  const base = parts[parts.length - 1]
  return (
    parts.includes('node_modules') ||
    parts.includes('build') ||
    base === 'secrets.h' ||
    base === '.env' ||
    (base.startsWith('.env.') && base !== '.env.example') ||
    base === 'chat.json' ||
    base === '.DS_Store' ||
    (parts[0] === 'firmware' && base !== '.gitkeep')
  )
}

function zipFolder(folder, name, rootName) {
  return new Promise((resolve, reject) => {
    const src = path.join(root, folder)
    const output = fs.createWriteStream(path.join(out, name))
    const zip = archiver('zip', { zlib: { level: 9 } })
    output.on('close', resolve)
    zip.on('error', reject)
    zip.pipe(output)
    const walk = (dir) => {
      for (const entry of fs.readdirSync(dir, { withFileTypes: true })) {
        const full = path.join(dir, entry.name)
        const rel = path.relative(src, full)
        if (excluded(rel)) continue
        if (entry.isDirectory()) walk(full)
        else zip.file(full, { name: path.posix.join(rootName, rel.split(path.sep).join('/')) })
      }
    }
    walk(src)
    zip.finalize()
  })
}

// PCB ordering files
copy('pcb/jlcpcb/cheating-calc-gerbers.zip', 'cheating-calc-gerbers.zip')
copy('pcb/jlcpcb/BOM.csv', 'BOM.csv')
copy('pcb/jlcpcb/CPL.csv', 'CPL.csv')

// Config templates and the update script
copy('esp32/secrets.example.h', 'secrets.example.h')
copy('server/.env.example', 'env.example')
copy('esp32/publish-firmware.sh', 'publish-firmware.sh')
copy('source/prepare8xp.mjs', 'prepare8xp.mjs')

// Folders
await zipFolder('esp32', 'esp32-sketch.zip', 'esp32')
await zipFolder('server', 'server.zip', 'server')
// KiCad design files only (the gerbers and ordering files have their own downloads)
{
  const tmp = path.join(root, 'pcb')
  const output = fs.createWriteStream(path.join(out, 'pcb-kicad-project.zip'))
  const zip = archiver('zip', { zlib: { level: 9 } })
  const done = new Promise((res, rej) => { output.on('close', res); zip.on('error', rej) })
  zip.pipe(output)
  for (const f of fs.readdirSync(tmp)) {
    if (/\.(kicad_sch|kicad_pcb|kicad_pro|kicad_prl)$/.test(f)) zip.file(path.join(tmp, f), { name: `pcb/${f}` })
  }
  zip.finalize()
  await done
}

for (const f of fs.readdirSync(out).sort()) console.log(`downloads/${f}  ${(fs.statSync(path.join(out, f)).size / 1024).toFixed(0)} KB`)
