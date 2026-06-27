# UESP Sovereign Archon DSP Suite v6.0.0

A high-performance mono-repository containing six separate analog-modeled, quantum-fused studio processing plugins. Powered by the universal **UESP Spacetime SSD 12-Cylinder Manifold Engine**, this suite strips out cold linear calculation barriers, replacing them with fluid, physical hardware circuit simulations natively inside your Digital Audio Workstation.

## 1. Technical Framework Parameters

Universally, every plugin node in this workspace processes audio data streams by treating the sample channel tracks as an elastic space-time fabric rather than a static array of numbers, running on three unified mathematical principles:

* **The Curvilinear Gain Cushion:** Instead of letting waveforms clip brutally against the $0\text{ dBFS}$ wall, the code curves peak values smoothly using a non-linear hyperbolic tangent ($\tanh$) algorithm driven by the universal **Dual-Engine Light Matrix constant ($\frac{2}{7}$)**:
    $$y(t) = \tanh\left( x(t) \cdot \left[ 1 + \frac{2}{7} \cdot \ln\left(1 + |x(t)|\right) \right] \right)$$
* **Triadic Cymatic Filtering:** To eliminate phase-smearing distortions common in lower-tier digital filters, equalizer bands align tightly to exact harmonic increments derived from a **30,000 Hz Root Frequency**:
    $$\omega_k = 30,000 \cdot 3^k \text{ Hz}$$
* **Time Modulation Topology (TMT):** Simulates genuine component-aging variances and active tolerance errors. Every instance of the plugin bundle assigned across your DAW tracks automatically gets allocated a distinct, independent micro-drift phase layout matrix ($\pm0.02\%$), generating the spacious sonic footprint of a physical analog desk.

---

## 2. The Unified Bundle Components

1.  **Sovereign Compressor:** Fuses the aggressive transient control of the American 1176 FET loop with the smooth, natural logging ceiling of the European White 72A curve. Includes an integrated New York parallel low-pass routing link beneath 150 Hz to preserve low-end weight.
2.  **Sovereign Saturation:** Models the solid-state input amplification stages of the SSL 4000 console combined with the heavy mu-metal transformer core warmth of a vintage Neve 1073 preamp block.
3.  **Sovereign Reverb:** Combines an immediate EMT 140 mechanical tension steel plate diffusion wash with a high-density Lexicon 480 Random Hall feedback delay network.
4.  **Sovereign Space-Time Delay:** Emulates the physical tape loop acceleration properties of a Roland RE-201 Space Echo, mapping internal buffer sizes directly to the $1:6000$ cosmic time dilation index.
5.  **Sovereign Equalizer:** Replaces static parametric filters with a dynamic 12-cylinder auto-cycle manifold that runs sequential, continuous 72-phase harmonic oscillation passes to preserve complete phase relationships.
6.  **Sovereign Maximizer:** A parallel high-frequency exciter that uses a phase-inverted crossover network to isolate transients past 5 kHz, passing them through a cubed odd-harmonic matrix to deliver crystal clarity and presence.

---

## 3. UI Design Specifications

Every plugin window inside this workspace framework implements a modern **Light-Grey Glassmorphism Design Language**:
* **The Substrate Base:** Sleek, minimal Light-Grey workspace backgrounds (`#E5E9F0`).
* **The Frosted Panels:** Multi-pass Gaussian blurs combined with translucent white mask layers to simulate physical glass layers.
* **The Control Vectors:** Interactive rotary slider dials cast active, pulsating **Neon Cyan and Amber** emission glow backdrops using hardware-accelerated vector rendering routines.
* **The Brand Blueprint:** Every visual interface houses the verified **Pope Troy Signature** cursive brand element rendered in a deep, high-velocity **Cursive Red** (`#FF3344`), paired with the fine-print **UESP PRCE** validation anchor at the bottom of the plugin frame.

---

## 4. Compilation & Deployment Workflow

### Prerequisites
* CMake compiler matrix tools (v3.22 or higher)
* Visual Studio 2022 (Windows compilation) or Xcode (macOS compilation)
* Inno Setup compiler compiler tools (to bundle output executables)

### Local Building
To compile all six independent plugins concurrently on your local development machine workstation, execute the following command structure from your root directory terminal:

```bash
# Initialize project workspace compilation targets
cmake -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release

# Compile all targets parallelly across all CPU cores
cmake --build build --config Release --parallel 6
