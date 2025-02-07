---
layout: default
title: RealViewOn
---

# ![logo](docs/assets/RealViewOn.png) ![header](docs/assets/Header.png) ![logo](docs/assets/RealViewOnOff.gif)

[![GitHub](https://img.shields.io/badge/GitHub-%23121011.svg?logo=github&logoColor=white&style=flat-square)](https://github.com/ianalexis/Real-View-On-Releases)
[![GitHub stars](https://img.shields.io/github/stars/ianalexis/Real-View-On-Releases?style=flat-square&logo=github)](https://github.com/ianalexis/Real-View-On-Releases/stargazers)
[![Version](https://img.shields.io/github/v/release/ianalexis/Real-View-On-Releases?color=darkgreen&label=Download%20Stable&style=flat-square)](https://github.com/ianalexis/Real-View-On-Releases/releases/latest/download/RealViewOn.7z)
[![Version](https://img.shields.io/github/v/release/ianalexis/Real-View-On-Releases?color=orangered&label=Download%20Pre-Release&style=flat-square&include_prereleases)](https://github.com/ianalexis/Real-View-On-Releases/releases)
[![Download](https://img.shields.io/badge/Download-Last%20Build-darkred.svg?style=flat-square&logo=download)](/RealViewOn.7z?raw=true)
[![C++](https://img.shields.io/badge/C++-%2300599C.svg?logo=c%2B%2B&logoColor=white&style=flat-square)](https://isocpp.org/)
[![Windows](https://custom-icon-badges.demolab.com/badge/Windows-0078D6?logo=windows11&logoColor=white&style=flat-square)](https://www.microsoft.com/windows/)

<div id="content-en">
  <!-- English content -->
  // ...existing code...
</div>

<div id="content-es" style="display:none;">
  <!-- Spanish content -->
  // ...existing code...
</div>

<script>
  function setLanguage(lang) {
    if (lang === 'es') {
      document.getElementById('content-en').style.display = 'none';
      document.getElementById('content-es').style.display = 'block';
    } else {
      document.getElementById('content-en').style.display = 'block';
      document.getElementById('content-es').style.display = 'none';
    }
  }

  const userLang = navigator.language || navigator.userLanguage;
  if (userLang.startsWith('es')) {
    setLanguage('es');
  } else {
    setLanguage('en');
  }
</script>
